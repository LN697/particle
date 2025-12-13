#include "kinematics.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

ParticleKinematics::ParticleKinematics(ParticleSystem& particles) : particles(particles) {
    gridWidth = static_cast<int>(std::ceil(boxWidth / CELL_SIZE));
    gridHeight = static_cast<int>(std::ceil(boxHeight / CELL_SIZE));
    grid.resize(gridWidth * gridHeight);
    
    std::cout << "[ParticleKinematics] Initialized with Grid: " 
              << gridWidth << "x" << gridHeight << std::endl;
}

void ParticleKinematics::init(const SimConfig& config) {
    // 1. Clear Data
    particles.posX.clear();
    particles.posY.clear();
    particles.velX.clear();
    particles.velY.clear();
    particles.planets.clear();

    numParticles = config.particleCount;

    particles.posX.reserve(numParticles);
    particles.posY.reserve(numParticles);
    particles.velX.reserve(numParticles);
    particles.velY.reserve(numParticles);

    float centerX = boxWidth / 2.0f;
    float centerY = boxHeight / 2.0f;

    // 2. Initialize Planets
    struct PlanetInit { float dist; float mass; float r; uint32_t col; };
    std::vector<PlanetInit> pInits = {
        { 40.0f,  200.0f, 3.0f, 0xFF5555AA }, // Mercury-ish
        { 70.0f,  400.0f, 4.0f, 0xFF00AAFF }, // Earth-ish
        { 120.0f, 3000.0f, 8.0f, 0xFFFFAA00 } // Jupiter-ish
    };

    for (const auto& pDef : pInits) {
        Planet p;
        float angle = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * M_PI;
        p.x = centerX + std::cos(angle) * pDef.dist;
        p.y = centerY + std::sin(angle) * pDef.dist;
        p.mass = pDef.mass;
        p.radius = pDef.r;
        p.color = pDef.col;

        float v = std::sqrt(config.starMass / pDef.dist);
        p.vx = -std::sin(angle) * v;
        p.vy = std::cos(angle) * v;

        particles.planets.push_back(p);
    }

    // 3. Initialize Asteroid Belt
    for (int i = 0; i < numParticles; ++i) {
        float angle = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * M_PI;
        
        float minR = 80.0f;
        float maxR = 110.0f;
        float rRand = static_cast<float>(rand()) / RAND_MAX;
        float radius = std::sqrt(rRand) * (maxR - minR) + minR;

        float px = centerX + std::cos(angle) * radius;
        float py = centerY + std::sin(angle) * radius;

        particles.posX.push_back(px);
        particles.posY.push_back(py);
        
        float dist = radius; 
        float orbitalSpeed = std::sqrt(config.starMass / dist);
        float variation = 1.0f + ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 0.15f;

        particles.velX.push_back(-std::sin(angle) * orbitalSpeed * variation);
        particles.velY.push_back(std::cos(angle) * orbitalSpeed * variation);
    }
}

void ParticleKinematics::step(SimConfig& config, float deltaTime) {
    // 1. Handle Spawning (Even if paused, so users can place objects)
    processUserSpawns(config);

    // 2. Pause Logic
    if (config.paused) return;

    // 3. Check for Reset
    if (static_cast<int>(particles.posX.size()) != config.particleCount) {
        init(config);
    }

    float subDt = deltaTime / static_cast<float>(config.substeps);
    
    for (int s = 0; s < config.substeps; ++s) {
        applyForces(config, subDt);
        updatePositions(config, subDt);
        if (config.enableCollisions) {
            resolveCollisionsGrid(config);
        }
        applyBoundaryConditions(config);
    }
}

void ParticleKinematics::processUserSpawns(SimConfig& config) {
    if (config.spawnClick) {
        float px = config.spawnX;
        float py = config.spawnY;
        float vx = config.spawnVelX;
        float vy = config.spawnVelY;

        // Auto Orbit Calculation
        if (config.spawnAutoOrbit) {
            float dx = px - config.starX;
            float dy = py - config.starY;
            float dist = std::sqrt(dx*dx + dy*dy);
            
            if (dist > 1.0f) {
                float orbSpeed = std::sqrt(config.starMass / dist);
                // Tangent vector: (-y, x) normalized
                vx = (-dy / dist) * orbSpeed;
                vy = (dx / dist) * orbSpeed;
            }
        }

        if (config.spawnType == 0) { // Planet
            Planet p;
            p.x = px; p.y = py;
            p.vx = vx; p.vy = vy;
            p.mass = config.spawnMass;
            p.radius = config.spawnRadius;
            
            // Convert float[3] to uint32 color
            uint8_t r = static_cast<uint8_t>(config.spawnColor[0] * 255);
            uint8_t g = static_cast<uint8_t>(config.spawnColor[1] * 255);
            uint8_t b = static_cast<uint8_t>(config.spawnColor[2] * 255);
            p.color = (0xFF << 24) | (r << 16) | (g << 8) | b;
            
            particles.planets.push_back(p);
            std::cout << "Spawned Planet at " << px << ", " << py << std::endl;
            
        } else { // Asteroid
            particles.posX.push_back(px);
            particles.posY.push_back(py);
            particles.velX.push_back(vx);
            particles.velY.push_back(vy);
            
            // Update the count so the loop doesn't reset
            config.particleCount++; 
            numParticles++;
        }

        // Consume the event
        config.spawnClick = false; 
    }
}

void ParticleKinematics::applyForces(const SimConfig& config, float dt) {
    float centerX = config.starX;
    float centerY = config.starY;
    float starMass = config.starMass;

    // --- 1. Update Planets Forces (N-Body Gravity) ---
    // Now iterating with index to allow nested comparison
    size_t numPlanets = particles.planets.size();
    
    for (size_t i = 0; i < numPlanets; ++i) {
        Planet& p1 = particles.planets[i];
        
        // A. Star Gravity (Star -> Planet)
        float dx = centerX - p1.x;
        float dy = centerY - p1.y;
        float distSq = dx*dx + dy*dy;
        float dist = std::sqrt(distSq);
        
        if (dist > 1.0f) {
            float force = starMass / distSq;
            p1.vx += (dx / dist) * force * dt;
            p1.vy += (dy / dist) * force * dt;
        }

        // B. Mutual Planet Gravity (Planet <-> Planet)
        // O(N^2) loop, but N is small (planets)
        for (size_t j = i + 1; j < numPlanets; ++j) {
            Planet& p2 = particles.planets[j];
            
            float pdx = p2.x - p1.x;
            float pdy = p2.y - p1.y;
            float pDistSq = pdx*pdx + pdy*pdy;
            float pDist = std::sqrt(pDistSq);
            
            // Softening radius to prevent singularity slingshots
            float minSep = p1.radius + p2.radius;
            if (pDist > minSep) {
                // F = G * m1 * m2 / r^2
                // We assume G=1 consistent with star logic
                // a1 = F/m1 = m2/r^2
                // a2 = F/m2 = m1/r^2
                
                float accel1 = p2.mass / pDistSq;
                float accel2 = p1.mass / pDistSq;
                
                float nx = pdx / pDist;
                float ny = pdy / pDist;
                
                // p1 is pulled towards p2
                p1.vx += nx * accel1 * dt;
                p1.vy += ny * accel1 * dt;
                
                // p2 is pulled towards p1 (Newton's 3rd Law)
                p2.vx -= nx * accel2 * dt;
                p2.vy -= ny * accel2 * dt;
            } else {
                // Optional: Simple overlap push (bounce)
                // Just to prevent them from sitting inside each other
                // but without momentum transfer for simplicity right now
            }
        }
    }

    // --- 2. Update Asteroid Forces ---
    for (int i = 0; i < numParticles; ++i) {
        // A. Star Gravity
        if (config.enableCentralGravity) {
            float dx = centerX - particles.posX[i];
            float dy = centerY - particles.posY[i];
            float distSq = dx*dx + dy*dy;
            float softeningSq = 5.0f; 
            float dist = std::sqrt(distSq + softeningSq);
            float force = starMass / (distSq + softeningSq); 

            particles.velX[i] += (dx / dist) * force * dt; 
            particles.velY[i] += (dy / dist) * force * dt;
        }

        // B. Planet Gravity
        for (const auto& p : particles.planets) {
            float dx = p.x - particles.posX[i];
            float dy = p.y - particles.posY[i];
            float distSq = dx*dx + dy*dy;
            
            if (distSq < 400.0f && distSq > (p.radius * p.radius)) {
                float dist = std::sqrt(distSq);
                float force = p.mass / distSq;
                particles.velX[i] += (dx / dist) * force * dt;
                particles.velY[i] += (dy / dist) * force * dt;
            }
        }
    }
}

void ParticleKinematics::updatePositions(const SimConfig& config, float dt) {
    for (auto& p : particles.planets) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
    }
    for (int i = 0; i < numParticles; ++i) {
        particles.posX[i] += particles.velX[i] * dt;
        particles.posY[i] += particles.velY[i] * dt;
        
        particles.velX[i] *= config.damping;
        particles.velY[i] *= config.damping;
    }
}

int ParticleKinematics::getGridIndex(float x, float y) {
    int cx = static_cast<int>(x / CELL_SIZE);
    int cy = static_cast<int>(y / CELL_SIZE);
    
    cx = std::max(0, std::min(cx, gridWidth - 1));
    cy = std::max(0, std::min(cy, gridHeight - 1));
    
    return cy * gridWidth + cx;
}

void ParticleKinematics::resolveCollisionsGrid(const SimConfig& config) {
    for (auto& cell : grid) cell.clear();
    for (int i = 0; i < numParticles; ++i) {
        grid[getGridIndex(particles.posX[i], particles.posY[i])].push_back(i);
    }

    float minDist = config.collisionRadius * 2.0f;
    float minDistSq = minDist * minDist;

    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            int cellIdx = y * gridWidth + x;
            int neighborOffsets[5][2] = {{0, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}};

            for (auto& offset : neighborOffsets) {
                int nx = x + offset[0];
                int ny = y + offset[1];

                if (nx >= 0 && nx < gridWidth && ny >= 0 && ny < gridHeight) {
                    int neighborIdx = ny * gridWidth + nx;
                    for (int i : grid[cellIdx]) {
                        for (int j : grid[neighborIdx]) {
                            if (i >= j) continue; 
                            float dx = particles.posX[i] - particles.posX[j];
                            float dy = particles.posY[i] - particles.posY[j];
                            float distSq = dx*dx + dy*dy;
                            if (distSq < minDistSq && distSq > 0.0001f) {
                                float dist = std::sqrt(distSq);
                                float overlap = (minDist - dist) * 0.5f;
                                float nx = dx / dist; float ny = dy / dist;
                                particles.posX[i] += nx * overlap; particles.posY[i] += ny * overlap;
                                particles.posX[j] -= nx * overlap; particles.posY[j] -= ny * overlap;
                                float dvx = particles.velX[i] - particles.velX[j];
                                float dvy = particles.velY[i] - particles.velY[j];
                                float velNormal = dvx * nx + dvy * ny;
                                if (velNormal < 0) {
                                    float impulse = -(1.0f + config.restitution) * velNormal * 0.5f;
                                    particles.velX[i] += impulse * nx; particles.velY[i] += impulse * ny;
                                    particles.velX[j] -= impulse * nx; particles.velY[j] -= impulse * ny;
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
    for (auto& p : particles.planets) {
        if (p.x < p.radius || p.x > boxWidth - p.radius) p.vx *= -1.0f;
        if (p.y < p.radius || p.y > boxHeight - p.radius) p.vy *= -1.0f;
    }

    for (int i = 0; i < numParticles; ++i) {
        if (particles.posX[i] < config.collisionRadius) {
            particles.posX[i] = config.collisionRadius; particles.velX[i] *= -config.restitution;
        } else if (particles.posX[i] > boxWidth - config.collisionRadius) {
            particles.posX[i] = boxWidth - config.collisionRadius; particles.velX[i] *= -config.restitution;
        }
        if (particles.posY[i] < config.collisionRadius) {
            particles.posY[i] = config.collisionRadius; particles.velY[i] *= -config.restitution;
        } else if (particles.posY[i] > boxHeight - config.collisionRadius) {
            particles.posY[i] = boxHeight - config.collisionRadius; particles.velY[i] *= -config.restitution;
        }
    }
}