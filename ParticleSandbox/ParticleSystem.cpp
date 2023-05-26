#include "ParticleSystem.h"

bool ParticleSystem::isPaused() {
    return this->pauseState;
}

void ParticleSystem::pause(bool pauseState) {
    this->pauseState = pauseState;
}

void ParticleSystem::nextFrame() {
    this->goNextFrame = true;
}

void ParticleSystem::add(Particle particle, int x, int y) {
    if (particle.getType() == air) {
        pixels[x][y] = particle;
        return;
    }

    if (!this->valid_position(x, y) || pixels[x][y].getType() != air)
        return;

    pixels[x][y] = particle;
}

bool ParticleSystem::valid_position(int xCoord, int yCoord) {
    return (xCoord >= 0 && xCoord < maxX && yCoord >= 0 && yCoord < maxY);
}

void ParticleSystem::update() {
    unsigned int gridWidth = maxX / 64;
    unsigned int gridHeight = maxY / 64;
    bool goNextFrameValid = goNextFrame;
    if (!pauseState || goNextFrameValid) {
        frameIndex++;
        frameIndex %= 2;
    }

    for (int offsetY = 0; offsetY <= 1; offsetY++) {
        for (int offsetX = 0; offsetX <= 1; offsetX++) {
            for (size_t ySquare = offsetY; ySquare < gridHeight; ySquare += 2) {
                for (size_t xSquare = offsetX; xSquare < gridWidth; xSquare += 2) {
                    threadPool.push([goNextFrameValid, xSquare, ySquare, this](int id)
                        {
                            for (int y = ySquare * 64 + 63; y >= (int)ySquare * 64; y--) {
                                for (int x = xSquare * 64; x < (1 + xSquare) * 64; x++) {
                                    Particle& particle = pixels[x][y];
                                    if (particle.shouldUpdate(frameIndex) || particle.getType() == ParticleType::air) {
                                        addParticleToDraw(x, y, particle.getColor());
                                        continue;
                                    }
                                    addParticleToDraw(x, y, particle.getColor());
                                    if (!pauseState || goNextFrameValid ) {
                                        particle.update(pixels, x, y);
                                    }
                                }
                            }
                            return; });
                    addChunkToDraw(xSquare, ySquare);
                }
            }
            while (threadPool.n_idle() != threadPool.size()) {}
        }
    }
    if (goNextFrameValid) {
        goNextFrame = false;
    }
}

void ParticleSystem::showChunkBorders(bool showBorders) {
    this->chunkBordersDraw = showBorders;
}

void ParticleSystem::addChunkToDraw(int xSquare, int ySquare) {
    int i = (xSquare + ySquare * 8) * 8;

    int x = xSquare * 64;
    int y = ySquare * 64;
    chunk_borders[i].position = sf::Vector2f(x, y);
    chunk_borders[i + 1].position = sf::Vector2f(x + 64, y);
    chunk_borders[i + 2].position = sf::Vector2f(x + 64, y);
    chunk_borders[i + 3].position = sf::Vector2f(x + 64, y + 64);
    chunk_borders[i + 4].position = sf::Vector2f(x + 64, y + 64);
    chunk_borders[i + 5].position = sf::Vector2f(x, y + 64);
    chunk_borders[i + 6].position = sf::Vector2f(x, y + 64);
    chunk_borders[i + 7].position = sf::Vector2f(x, y);
}

void ParticleSystem::addParticleToDraw(int x, int y, sf::Color particle_color) {
    int i = (y * 512 + x) * 4;

    screen_particles[i].position = sf::Vector2f(x, y);
    screen_particles[i + 1].position = sf::Vector2f(x + 1, y);
    screen_particles[i + 2].position = sf::Vector2f(x + 1, y + 1);
    screen_particles[i + 3].position = sf::Vector2f(x, y + 1);

    screen_particles[i].color = particle_color;
    screen_particles[i + 1].color = particle_color;
    screen_particles[i + 2].color = particle_color;
    screen_particles[i + 3].color = particle_color;
};

void ParticleSystem::draw(sf::RenderTarget& target, sf::RenderStates states) const {
    states.transform *= getTransform();

    states.texture = NULL;

    target.draw(&screen_particles[0], screen_particles.size(), sf::Quads);
    if (this->chunkBordersDraw)
        target.draw(&chunk_borders[0], chunk_borders.size(), sf::Lines);
}

DebugParticleInfo ParticleSystem::getParticleInfoFromPos(int xCoord, int yCoord) {
    if (!valid_position(xCoord, yCoord)) {
        return { Particle(ParticleType::air) , -1 , -1 };
    }
    return { pixels[xCoord][yCoord], xCoord, yCoord };
}