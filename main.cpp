#include "kinematics.hpp"
#include "renderer.hpp"
#include <iostream>
#include <csignal>
#include <vector> // Add this include for std::vector

volatile std::sig_atomic_t g_signal_received = 0;
void signal_handler(int signal) { g_signal_received = signal; }

// Function to render particles
void render(Renderer& renderer, const ParticleSystem& particles, int screenWidth, int screenHeight, float boxWidth, float boxHeight) {
    std::vector<uint32_t> buffer(screenWidth * screenHeight, 0xFF000000); // Black background

    for (size_t i = 0; i < particles.posX.size(); ++i) {
        // Map particle coordinates to screen coordinates
        int pixelX = static_cast<int>((particles.posX[i] / boxWidth) * screenWidth);
        int pixelY = static_cast<int>((particles.posY[i] / boxHeight) * screenHeight);

        // Ensure pixel coordinates are within bounds
        if (pixelX >= 0 && pixelX < screenWidth && pixelY >= 0 && pixelY < screenHeight) {
            buffer[pixelY * screenWidth + pixelX] = 0xFFFFFFFF; // White particle
        }
    }
    renderer.draw(buffer);
}

int main(int argc, char* argv[]) {
    const int screenWidth = 256;
    const int screenHeight = 256;
    const float simulationBoxWidth = 100.0f; // Assuming these values from kinematics.hpp
    const float simulationBoxHeight = 100.0f; // Assuming these values from kinematics.hpp

    Renderer renderer;
    if (!renderer.init("Particle Simulation", screenWidth, screenHeight, 2)) {
        std::cerr << "[Error] Failed to initialize the renderer." << std::endl;
        return -1;
    }

    ParticleSystem particles;
    ParticleKinematics kinematics(particles);

    kinematics.init(100);

    // Initial render before simulation steps
    render(renderer, particles, screenWidth, screenHeight, simulationBoxWidth, simulationBoxHeight);


    std::signal(SIGINT, signal_handler);

    while (g_signal_received == 0) {
        if (!renderer.handleEvents()) break;

        kinematics.step(); // Move the step inside the loop for continuous simulation
        render(renderer, particles, screenWidth, screenHeight, simulationBoxWidth, simulationBoxHeight);
    }

    return 0;
}