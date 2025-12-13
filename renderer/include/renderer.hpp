#pragma once

#include <SDL2/SDL.h>
#include <vector>
#include <cstdint>
#include "common.hpp"

// Forward declaration for ImGui
struct SDL_Window;
struct SDL_Renderer;

class Renderer {
    public:
        Renderer();
        ~Renderer();

        bool init(const char* title, int width, int height, int scale);
        void shutdown();
        
        bool handleEvents(SimConfig& config);
        void render(const std::vector<uint32_t>& buffer, SimConfig& config);

    private:
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        SDL_Texture* texture = nullptr;
        
        // Track dimensions to avoid hardcoding
        int renderWidth = 0;
        int renderHeight = 0;
        
        // UI State
        bool showUI = true;
};