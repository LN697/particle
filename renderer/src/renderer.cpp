#include "renderer.hpp"
#include <iostream>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

Renderer::Renderer() {}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::init(const char* title, int width, int height, int scale) {
    renderWidth = width;
    renderHeight = height;

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0) return false;
    
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                              width * scale, height * scale, window_flags);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    SDL_RenderSetLogicalSize(renderer, width, height);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui::StyleColorsDark();

    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    return true;
}

void Renderer::shutdown() {
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    if (texture) SDL_DestroyTexture(texture);
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Renderer::handleEvents(SimConfig& config) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL2_ProcessEvent(&event);
        
        if (event.type == SDL_QUIT) return false;
        
        // --- 1. Pause on Esc ---
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            config.paused = !config.paused;
        }
        
        // --- 2. Handle Spawning (If ImGui is not using the mouse) ---
        if (!ImGui::GetIO().WantCaptureMouse) {
            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                // Convert Screen Coords -> Simulation Coords
                int logicalX, logicalY;
                SDL_GetMouseState(&logicalX, &logicalY);
                
                // Adjust for scale
                // logicalX is 0..1024, SIM_WIDTH is 0..300
                float scaleX = SIM_WIDTH / static_cast<float>(renderWidth);
                float scaleY = SIM_HEIGHT / static_cast<float>(renderHeight);
                
                config.spawnX = logicalX * scaleX;
                config.spawnY = logicalY * scaleY;
                config.spawnClick = true; // Signal kinematics to spawn next frame
            }
        }
    }
    return true;
}

void Renderer::render(const std::vector<uint32_t>& buffer, SimConfig& config) {
    SDL_UpdateTexture(texture, nullptr, buffer.data(), renderWidth * sizeof(uint32_t));
    
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    if (showUI) {
        ImGui::Begin("Cosmic Controls");
        
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        
        // --- Pause Control ---
        ImGui::Separator();
        // Checkbox returns true if clicked, toggling the bool
        ImGui::Checkbox("Pause Simulation (Esc)", &config.paused);
        if (config.paused) {
            ImGui::SameLine();
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "[PAUSED]");
        }

        // --- Object Spawner ---
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Object Spawner (Click to Place)");
        
        ImGui::RadioButton("Planet", &config.spawnType, 0); ImGui::SameLine();
        ImGui::RadioButton("Asteroid", &config.spawnType, 1);
        
        if (config.spawnType == 0) { // Planet Settings
            ImGui::SliderFloat("Mass", &config.spawnMass, 50.0f, 5000.0f);
            ImGui::SliderFloat("Radius", &config.spawnRadius, 1.0f, 20.0f);
            ImGui::ColorEdit3("Color", config.spawnColor);
        }
        
        // Velocity Settings
        ImGui::Checkbox("Auto Orbit Velocity", &config.spawnAutoOrbit);
        if (!config.spawnAutoOrbit) {
            ImGui::SliderFloat("Vel X", &config.spawnVelX, -50.0f, 50.0f);
            ImGui::SliderFloat("Vel Y", &config.spawnVelY, -50.0f, 50.0f);
        } else {
            ImGui::TextDisabled("Velocity calculated automatically");
        }

        ImGui::Separator();
        ImGui::Text("System Config");
        ImGui::SliderInt("Particles", &config.particleCount, 100, 10000);
        ImGui::SliderFloat("Star Mass", &config.starMass, 100.0f, 20000.0f);
        
        ImGui::End();
    }

    ImGui::Render();
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    SDL_RenderPresent(renderer);
}