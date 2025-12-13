#pragma once

#include "particle.hpp"
#include "common.hpp"
#include <vector>

class ParticleKinematics {
    private:
        ParticleSystem& particles;
        int numParticles = 0;

        // --- Box dimensions ---
        const float boxWidth = SIM_WIDTH;
        const float boxHeight = SIM_HEIGHT;

        // --- Spatial Grid Optimization ---
        static constexpr float CELL_SIZE = 2.5f; 
        int gridWidth;
        int gridHeight;
        std::vector<std::vector<int>> grid;

    public:
        ParticleKinematics(ParticleSystem& particles);

        void init(const SimConfig& config);
        // Note: step is no longer const because it can modify particles via spawn
        void step(SimConfig& config, float deltaTime);

    private:
        void processUserSpawns(SimConfig& config); // New method
        void updatePositions(const SimConfig& config, float dt);
        void applyForces(const SimConfig& config, float dt);
        void resolveCollisionsGrid(const SimConfig& config);
        void applyBoundaryConditions(const SimConfig& config);
        
        int getGridIndex(float x, float y);
};