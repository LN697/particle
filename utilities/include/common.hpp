#pragma once

#include <cstdint>

#define KINEMATICS

// Configuration struct to control simulation parameters from UI
struct SimConfig {
    int particleCount = 1000;
    
    // Global Forces
    float gravityX = 0.0f;
    float gravityY = 9.8f;      // Standard gravity
    bool enableGravity = true;
    
    // Particle Properties
    float damping = 0.99f;      // Air resistance
    float restitution = 0.8f;   // Wall bounce energy loss
    float collisionRadius = 1.0f;
    bool enableCollisions = true;
    
    // Interaction
    bool enableMouseAttraction = false;
    float mouseX = 0.0f;
    float mouseY = 0.0f;
    float mouseStrength = 50.0f;
    
    // Advanced Physics
    bool enableInterParticleGravity = false;
    float interParticleG = 0.05f; // Constant for attraction between particles
    int substeps = 4;             // Sub-steps for stability
};