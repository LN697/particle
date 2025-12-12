#include "kinematics.hpp"
#include <iostream>

ParticleKinematics::ParticleKinematics(ParticleSystem& particles) : particles(particles) {
    std::cout << "[ParticleKinematics] Initialized." << std::endl;
}

void ParticleKinematics::init(int count) {
    particles.posX.clear();
    particles.posY.clear();
    particles.velX.clear();
    particles.velY.clear();

    numParticles = count;

    // Example: Initialize 100 particles at random positions and velocities
    for (int i = 0; i < numParticles; ++i) {
        particles.posX.push_back(static_cast<float>(rand()) / RAND_MAX * boxWidth);
        particles.posY.push_back(static_cast<float>(rand()) / RAND_MAX * boxHeight);

        particles.velX.push_back((static_cast<float>(rand()) / RAND_MAX - 0.5f) * 50.0f);
        particles.velY.push_back((static_cast<float>(rand()) / RAND_MAX - 0.5f) * 50.0f);
    }
}

void ParticleKinematics::step() {
    for (int i = 0; i < numParticles; ++i) {
        checkCollisions();
        checkForce();

        // Update positions based on velocities
        particles.posX[i] += particles.velX[i] * dt;
        particles.posY[i] += particles.velY[i] * dt;

        // Handle boundary conditions (simple wrap-around)
        if (particles.posX[i] < 0) particles.posX[i] += boxWidth;
        if (particles.posX[i] >= boxWidth) particles.posX[i] -= boxWidth;
        if (particles.posY[i] < 0) particles.posY[i] += boxHeight;
        if (particles.posY[i] >= boxHeight) particles.posY[i] -= boxHeight;
    }
}

void ParticleKinematics::checkCollisions() {
    // Simple collision detection and response between particles
    for (int i = 0; i < numParticles; ++i) {
        for (int j = i + 1; j < numParticles; ++j) {
            float dx = particles.posX[i] - particles.posX[j];
            float dy = particles.posY[i] - particles.posY[j];
            float distanceSquared = dx * dx + dy * dy;
            float collisionDistance = 1.125f;

            if (distanceSquared < collisionDistance * collisionDistance) {
                // Simple elastic collision response
                std::swap(particles.velX[i], particles.velX[j]);
                std::swap(particles.velY[i], particles.velY[j]);
            }
        }
    }
}

void ParticleKinematics::checkForce() {

}