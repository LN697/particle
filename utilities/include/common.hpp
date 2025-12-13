#pragma once

#include <cstdint>

#define KINEMATICS

// Global Simulation Constants
constexpr float SIM_WIDTH = 300.0f;
constexpr float SIM_HEIGHT = 300.0f;

enum class BodyType {
    Planet = 0,
    Asteroid = 1
};

// Configuration struct to control simulation parameters from UI
struct SimConfig {
    // --- System State ---
    bool paused = false;
    int particleCount = 4000;
    
    // --- Global Forces ---
    bool enableGravity = false; 
    float gravityX = 0.0f;
    float gravityY = 9.8f;
    
    // --- Central Star ---
    bool enableCentralGravity = true;
    float starMass = 10000.0f; 
    float starX = SIM_WIDTH / 2.0f;
    float starY = SIM_HEIGHT / 2.0f;
    
    // --- Physics Properties ---
    float damping = 1.0f;       
    float restitution = 0.6f;   
    float collisionRadius = 0.3f; 
    bool enableCollisions = true;
    
    // --- Advanced Physics ---
    bool enableInterParticleGravity = false;
    float interParticleG = 0.05f; 
    int substeps = 8;             

    // --- Object Spawner Settings (Controlled by ImGui) ---
    int spawnType = 0; // 0 = Planet, 1 = Asteroid
    float spawnMass = 200.0f;
    float spawnRadius = 4.0f;
    float spawnColor[3] = { 0.0f, 0.5f, 1.0f }; // RGB
    bool spawnAutoOrbit = true; // Auto-calculate circular orbit velocity
    float spawnVelX = 0.0f;
    float spawnVelY = 0.0f;

    // --- Input Events (Renderer writes, Kinematics reads) ---
    bool spawnClick = false;
    float spawnX = 0.0f;
    float spawnY = 0.0f;
};