#include "kinematics.hpp"
#include "renderer.hpp"
#include "common.hpp"
#include <iostream>
#include <vector>

// Helper to fill buffer from particles
void updateBuffer(std::vector<uint32_t>& buffer, const ParticleSystem& particles, int screenWidth, int screenHeight, float boxWidth, float boxHeight) {
    std::fill(buffer.begin(), buffer.end(), 0xFF000000); // Clear screen

    for (size_t i = 0; i < particles.posX.size(); ++i) {
        int pixelX = static_cast<int>((particles.posX[i] / boxWidth) * screenWidth);
        int pixelY = static_cast<int>((particles.posY[i] / boxHeight) * screenHeight);

        if (pixelX >= 0 && pixelX < screenWidth && pixelY >= 0 && pixelY < screenHeight) {
            // Simple color based on velocity magnitude
            float v = std::abs(particles.velX[i]) + std::abs(particles.velY[i]);
            uint8_t r = std::min(255, static_cast<int>(v * 5));
            uint8_t g = std::min(255, static_cast<int>(100 + v * 2));
            uint8_t b = 255;
            
            uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
            buffer[pixelY * screenWidth + pixelX] = color;
        }
    }
}

int main(int, char**) {
    const int screenWidth = 256;
    const int screenHeight = 256;
    
    // Initialize Config
    SimConfig config;
    config.particleCount = 1000;

    Renderer renderer;
    if (!renderer.init("Particle Physics Simulator", screenWidth, screenHeight, 3)) {
        std::cerr << "[Error] Failed to initialize renderer." << std::endl;
        return -1;
    }

    ParticleSystem particles;
    ParticleKinematics kinematics(particles);

    kinematics.init(config);

    std::vector<uint32_t> buffer(screenWidth * screenHeight);

    // Main Loop
    while (true) {
        // Handle input (including ImGui)
        if (!renderer.handleEvents(config)) break;

        // Physics Step
        kinematics.step(config, 0.016f); // Assume 60hz for now

        // Render
        updateBuffer(buffer, particles, screenWidth, screenHeight, 100.0f, 100.0f);
        renderer.render(buffer, config);
    }

    return 0;
}