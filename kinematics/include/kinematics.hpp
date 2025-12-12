#pragma once

#include "particle.hpp"
#include "common.hpp"
#include <vector>

class ParticleKinematics {
    private:
        ParticleSystem& particles;
        int numParticles = 0;

        // --- Box dimensions ---
        const float boxWidth = 100.0f;
        const float boxHeight = 100.0f;

        // --- Spatial Grid Optimization ---
        // Cell size should be >= diameter of largest particle (2.0f typically)
        static constexpr float CELL_SIZE = 2.5f; 
        int gridWidth;
        int gridHeight;
        std::vector<std::vector<int>> grid;

        float dt = 0.016f; // ~60 FPS fixed step

    public:
        ParticleKinematics(ParticleSystem& particles);

        void init(const SimConfig& config);
        void step(const SimConfig& config, float deltaTime);

    private:
        void updatePositions(const SimConfig& config, float dt);
        void applyForces(const SimConfig& config);
        void resolveCollisions(const SimConfig& config);
        void resolveCollisionsGrid(const SimConfig& config);
        void applyBoundaryConditions(const SimConfig& config);
        
        // Helper to get grid index
        int getGridIndex(float x, float y);
};