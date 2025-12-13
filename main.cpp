#include "kinematics.hpp"
#include "renderer.hpp"
#include "common.hpp"
#include <iostream>
#include <vector>
#include <cmath>

// Helper to fill buffer from particles
void updateBuffer(std::vector<uint32_t>& buffer, const ParticleSystem& particles, int screenWidth, int screenHeight) {
    // Fast clear (black space)
    std::fill(buffer.begin(), buffer.end(), 0xFF000000); 

    // Draw Star at center (Using SIM_WIDTH/HEIGHT coordinates)
    // Map simulation space (0..300) to screen space (0..1024)
    float scaleX = screenWidth / SIM_WIDTH;
    float scaleY = screenHeight / SIM_HEIGHT;

    int starPixelX = static_cast<int>((SIM_WIDTH / 2.0f) * scaleX);
    int starPixelY = static_cast<int>((SIM_HEIGHT / 2.0f) * scaleY);
    int starRadius = 8;
    
    for(int y = -starRadius; y <= starRadius; y++) {
        for(int x = -starRadius; x <= starRadius; x++) {
            if (x*x + y*y <= starRadius*starRadius) {
                int px = starPixelX + x;
                int py = starPixelY + y;
                if(px >= 0 && px < screenWidth && py >= 0 && py < screenHeight) {
                    buffer[py * screenWidth + px] = 0xFFFFFF00; // Yellow sun
                }
            }
        }
    }

    // Draw Planets
    for (const auto& p : particles.planets) {
        int px = static_cast<int>(p.x * scaleX);
        int py = static_cast<int>(p.y * scaleY);
        int pr = static_cast<int>(p.radius * (scaleX / 5.0f)); // Scale radius for visibility
        if (pr < 2) pr = 2;

        for(int y = -pr; y <= pr; y++) {
            for(int x = -pr; x <= pr; x++) {
                if (x*x + y*y <= pr*pr) {
                    int dpx = px + x;
                    int dpy = py + y;
                    if(dpx >= 0 && dpx < screenWidth && dpy >= 0 && dpy < screenHeight) {
                        buffer[dpy * screenWidth + dpx] = p.color;
                    }
                }
            }
        }
    }

    // Draw Particles
    for (size_t i = 0; i < particles.posX.size(); ++i) {
        int pixelX = static_cast<int>(particles.posX[i] * scaleX);
        int pixelY = static_cast<int>(particles.posY[i] * scaleY);

        if (pixelX >= 0 && pixelX < screenWidth && pixelY >= 0 && pixelY < screenHeight) {
            // Color based on velocity
            float vx = particles.velX[i];
            float vy = particles.velY[i];
            float vSq = vx*vx + vy*vy;
            
            float t = std::min(1.0f, vSq / 500.0f); // Adjusted max speed for color
            
            uint8_t r = static_cast<uint8_t>(t * 255);
            uint8_t g = static_cast<uint8_t>((1.0f - t) * 150 + 50);
            uint8_t b = static_cast<uint8_t>((1.0f - t) * 255);
            
            uint32_t color = (0xFF << 24) | (r << 16) | (g << 8) | b;
            buffer[pixelY * screenWidth + pixelX] = color;
        }
    }
}

int main(int, char**) {
    const int screenWidth = 1024;
    const int screenHeight = 1024;
    
    // Initialize Config
    SimConfig config;
    config.particleCount = 5000;

    Renderer renderer;
    if (!renderer.init("Cosmic Simulation", screenWidth, screenHeight, 1)) {
        std::cerr << "[Error] Failed to initialize renderer." << std::endl;
        return -1;
    }

    ParticleSystem particles;
    ParticleKinematics kinematics(particles);

    kinematics.init(config);

    std::vector<uint32_t> buffer(screenWidth * screenHeight);

    // Main Loop
    while (true) {
        if (!renderer.handleEvents(config)) break;

        kinematics.step(config, 0.016f); // 60hz

        updateBuffer(buffer, particles, screenWidth, screenHeight);
        renderer.render(buffer, config);
    }

    return 0;
}