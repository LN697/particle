#include "renderer.hpp"
#include <iostream>

// Include ImGui
// NOTE: Make sure the vendor/imgui folder is in your include path
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

Renderer::Renderer() {}

Renderer::~Renderer() {
    shutdown();
}

bool Renderer::init(const char* title, int width, int height, int scale) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) < 0) return false;
    
    // Create window with OpenGL context flag (often needed for ImGui even with SDL renderer)
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                              width * scale, height * scale, window_flags);
    if (!window) return false;

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) return false;

    SDL_RenderSetLogicalSize(renderer, width, height);
    texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
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
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return false;
        
        // Mouse input for attraction
        if (event.type == SDL_MOUSEMOTION) {
            // Need to map window coords to simulation coords
            int logicalX, logicalY;
            SDL_GetMouseState(&logicalX, &logicalY); // Gets window coords
            
            // Adjust for logical render size if necessary
            int w, h;
            SDL_GetWindowSize(window, &w, &h);
            float scaleX = 256.0f / w;
            float scaleY = 256.0f / h;
            
            config.mouseX = logicalX * scaleX;
            config.mouseY = logicalY * scaleY;
        }
        
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            config.enableMouseAttraction = true;
        }
        if (event.type == SDL_MOUSEBUTTONUP) {
            config.enableMouseAttraction = false;
        }
    }
    return true;
}

void Renderer::render(const std::vector<uint32_t>& buffer, SimConfig& config) {
    // 1. Draw Simulation to Texture
    SDL_UpdateTexture(texture, nullptr, buffer.data(), 256 * sizeof(uint32_t));
    
    // Start the Dear ImGui frame
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    // 2. Define UI Window
    if (showUI) {
        ImGui::Begin("Physics Controls");
        
        ImGui::Text("Simulation Stats");
        ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        
        ImGui::Separator();
        ImGui::Text("System");
        ImGui::SliderInt("Particles", &config.particleCount, 100, 5000);
        ImGui::SliderInt("Substeps", &config.substeps, 1, 16);
        
        ImGui::Separator();
        ImGui::Text("Forces");
        ImGui::Checkbox("Enable Gravity", &config.enableGravity);
        ImGui::SliderFloat("Gravity X", &config.gravityX, -20.0f, 20.0f);
        ImGui::SliderFloat("Gravity Y", &config.gravityY, -20.0f, 20.0f);
        ImGui::Checkbox("Inter-Particle Gravity", &config.enableInterParticleGravity);
        if (config.enableInterParticleGravity) {
            ImGui::SliderFloat("G Constant", &config.interParticleG, 0.01f, 1.0f);
        }

        ImGui::Separator();
        ImGui::Text("Properties");
        ImGui::SliderFloat("Damping", &config.damping, 0.90f, 1.0f);
        ImGui::SliderFloat("Restitution", &config.restitution, 0.0f, 1.2f);
        ImGui::Checkbox("Collisions", &config.enableCollisions);
        
        ImGui::Separator();
        ImGui::Text("Interaction");
        ImGui::SliderFloat("Mouse Strength", &config.mouseStrength, 10.0f, 500.0f);

        ImGui::End();
    }

    // 3. Render
    ImGui::Render();
    SDL_RenderCopy(renderer, texture, nullptr, nullptr);
    
    // FIX: Passed the renderer instance as the second argument
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
    
    SDL_RenderPresent(renderer);
}