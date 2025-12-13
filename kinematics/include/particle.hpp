#pragma once

#include <vector>
#include <cstdint>

struct Planet {
    float x, y;
    float vx, vy;
    float mass;
    float radius;
    uint32_t color;
};

struct ParticleSystem {
    // Structure of Arrays (SoA) for efficient cache access of asteroids
    std::vector<float> posX;
    std::vector<float> posY;
    std::vector<float> velX;
    std::vector<float> velY;

    // Array of Structures (AoS) for planets (low count, cache irrelevant)
    std::vector<Planet> planets;
};