#pragma once

#include "particle.hpp"

class ParticleKinematics {
    private:
        ParticleSystem& particles;
        int numParticles = 100;

        // --- Box dimensions ---
        const float boxWidth = 100.0f;
        const float boxHeight = 100.0f;

        float dt = 0.01f;

    public:
        ParticleKinematics(ParticleSystem& particles);

        void step();
        void init(int count);

        void checkCollisions();
        void checkForce();
};