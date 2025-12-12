#include "kinematics.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

ParticleKinematics::ParticleKinematics(ParticleSystem& particles) : particles(particles) {
    // Initialize grid dimensions based on box size
    gridWidth = static_cast<int>(std::ceil(boxWidth / CELL_SIZE));
    gridHeight = static_cast<int>(std::ceil(boxHeight / CELL_SIZE));
    grid.resize(gridWidth * gridHeight);
    
    std::cout << "[ParticleKinematics] Initialized with Grid: " 
              << gridWidth << "x" << gridHeight << std::endl;
}

void ParticleKinematics::init(const SimConfig& config) {
    particles.posX.clear();
    particles.posY.clear();
    particles.velX.clear();
    particles.velY.clear();

    numParticles = config.particleCount;

    // Reserve memory to avoid reallocations
    particles.posX.reserve(numParticles);
    particles.posY.reserve(numParticles);
    particles.velX.reserve(numParticles);
    particles.velY.reserve(numParticles);

    for (int i = 0; i < numParticles; ++i) {
        particles.posX.push_back(static_cast<float>(rand()) / RAND_MAX * (boxWidth - 2.0f) + 1.0f);
        particles.posY.push_back(static_cast<float>(rand()) / RAND_MAX * (boxHeight - 2.0f) + 1.0f);
        
        // Give random initial velocity
        particles.velX.push_back((static_cast<float>(rand()) / RAND_MAX - 0.5f) * 20.0f);
        particles.velY.push_back((static_cast<float>(rand()) / RAND_MAX - 0.5f) * 20.0f);
    }
}

void ParticleKinematics::step(const SimConfig& config, float deltaTime) {
    // Check if particle count changed in UI
    if (static_cast<int>(particles.posX.size()) != config.particleCount) {
        init(config);
    }

    // Sub-stepping for stability
    float subDt = deltaTime / static_cast<float>(config.substeps);
    
    for (int s = 0; s < config.substeps; ++s) {
        applyForces(config);
        updatePositions(config, subDt);
        if (config.enableCollisions) {
            resolveCollisionsGrid(config);
        }
        applyBoundaryConditions(config);
    }
}

void ParticleKinematics::applyForces(const SimConfig& config) {
    for (int i = 0; i < numParticles; ++i) {
        // 1. Global Gravity
        if (config.enableGravity) {
            particles.velX[i] += config.gravityX * 0.1f; 
            particles.velY[i] += config.gravityY * 0.1f;
        }

        // 2. Mouse Attraction
        if (config.enableMouseAttraction) {
            float dx = (config.mouseX / 256.0f * boxWidth) - particles.posX[i]; // Map screen to box
            float dy = (config.mouseY / 256.0f * boxHeight) - particles.posY[i];
            float distSq = dx*dx + dy*dy + 0.1f; // Avoid divide by zero
            float force = config.mouseStrength / std::sqrt(distSq);
            
            particles.velX[i] += dx * force * 0.01f;
            particles.velY[i] += dy * force * 0.01f;
        }

        // 3. Inter-particle Gravity (Brute force O(N^2) - Careful!)
        if (config.enableInterParticleGravity) {
            for (int j = 0; j < numParticles; ++j) {
                if (i == j) continue;
                float dx = particles.posX[j] - particles.posX[i];
                float dy = particles.posY[j] - particles.posY[i];
                float distSq = dx*dx + dy*dy;
                
                // Softening parameter to prevent infinity
                if (distSq > 0.5f && distSq < 100.0f) { 
                    float force = config.interParticleG / distSq;
                    particles.velX[i] += dx * force;
                    particles.velY[i] += dy * force;
                }
            }
        }
    }
}

void ParticleKinematics::updatePositions(const SimConfig& config, float dt) {
    for (int i = 0; i < numParticles; ++i) {
        particles.posX[i] += particles.velX[i] * dt;
        particles.posY[i] += particles.velY[i] * dt;
        
        // Apply damping
        particles.velX[i] *= config.damping;
        particles.velY[i] *= config.damping;
    }
}

int ParticleKinematics::getGridIndex(float x, float y) {
    int cx = static_cast<int>(x / CELL_SIZE);
    int cy = static_cast<int>(y / CELL_SIZE);
    
    // Clamp to valid grid coords
    cx = std::max(0, std::min(cx, gridWidth - 1));
    cy = std::max(0, std::min(cy, gridHeight - 1));
    
    return cy * gridWidth + cx;
}

void ParticleKinematics::resolveCollisionsGrid(const SimConfig& config) {
    // 1. Clear grid
    for (auto& cell : grid) {
        cell.clear();
    }

    // 2. Populate grid
    for (int i = 0; i < numParticles; ++i) {
        int idx = getGridIndex(particles.posX[i], particles.posY[i]);
        grid[idx].push_back(i);
    }

    // 3. Check collisions
    float minDist = config.collisionRadius * 2.0f;
    float minDistSq = minDist * minDist;

    // Loop through all cells
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            int cellIdx = y * gridWidth + x;
            
            // Check particles in current cell against particles in neighbor cells
            // We only need to check 4 neighbors (plus current) to avoid duplicates:
            // Current, Right, Bottom-Left, Bottom, Bottom-Right
            int neighborOffsets[5][2] = {
                {0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}
            };

            for (auto& offset : neighborOffsets) {
                int nx = x + offset[0];
                int ny = y + offset[1];

                if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                    int neighborIdx = ny * gridWidth + nx;
                    
                    for (int i : grid[cellIdx]) {
                        for (int j : grid[neighborIdx]) {
                            if (i >= j) continue; // Avoid self and duplicate checks

                            float dx = particles.posX[i] - particles.posX[j];
                            float dy = particles.posY[i] - particles.posY[j];
                            float distSq = dx*dx + dy*dy;

                            if (distSq < minDistSq && distSq > 0.0001f) {
                                float dist = std::sqrt(distSq);
                                float overlap = (minDist - dist) * 0.5f;
                                float nx = dx / dist;
                                float ny = dy / dist;

                                // Separate particles
                                particles.posX[i] += nx * overlap;
                                particles.posY[i] += ny * overlap;
                                particles.posX[j] -= nx * overlap;
                                particles.posY[j] -= ny * overlap;

                                // Resolve Velocity (Elastic)
                                float dvx = particles.velX[i] - particles.velX[j];
                                float dvy = particles.velY[i] - particles.velY[j];
                                float velAlongNormal = dvx * nx + dvy * ny;

                                if (velAlongNormal < 0) { // Moving towards each other
                                    float jImpulse = -(1.0f + 1.0f) * velAlongNormal * 0.5f; // Mass assumed 1
                                    
                                    particles.velX[i] += jImpulse * nx;
                                    particles.velY[i] += jImpulse * ny;
                                    particles.velX[j] -= jImpulse * nx;
                                    particles.velY[j] -= jImpulse * ny;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void ParticleKinematics::applyBoundaryConditions(const SimConfig& config) {
    for (int i = 0; i < numParticles; ++i) {
        if (particles.posX[i] < config.collisionRadius) {
            particles.posX[i] = config.collisionRadius;
            particles.velX[i] *= -config.restitution;
        } else if (particles.posX[i] > boxWidth - config.collisionRadius) {
            particles.posX[i] = boxWidth - config.collisionRadius;
            particles.velX[i] *= -config.restitution;
        }

        if (particles.posY[i] < config.collisionRadius) {
            particles.posY[i] = config.collisionRadius;
            particles.velY[i] *= -config.restitution;
        } else if (particles.posY[i] > boxHeight - config.collisionRadius) {
            particles.posY[i] = boxHeight - config.collisionRadius;
            particles.velY[i] *= -config.restitution;
        }
    }
}