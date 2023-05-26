#pragma once
#include "ctpl_stl.h"
#include <SFML/Graphics.hpp>
#include <thread>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

enum ParticleType { air, solid, sand, liquid, gas, fire };

class Particle {
public:
    Particle(ParticleType type, float density = 1.f);
    sf::Color getColor() const;
    ParticleType getType() const;
    void setDensity(float density);
    void setLifeTime(int lifeTime);
    void setFlammability(bool flammable);
    void setFuelTime(int fuelTime);
    virtual ~Particle() {};
    virtual void update(std::vector<std::vector<Particle>>& pixels, int x, int y);
    bool shouldUpdate(int frame_index);
    int getUpdateIndex();
    static bool validPosition(int boundsX, int boundsY, int xCoord, int yCoord);
private:
    void updateSand(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY);
    void updateLiquid(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY);
    void updateGas(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY);
    void fireSpread(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY);
    void updateFire(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY);
    ParticleType particleType = ParticleType::solid;
    int hasBeenUpdated = 0;
    int currentChunk = -1;
    float density = 1;
    float velocity = 0;
    bool perishable = false;
    int lifeTime = -1;
    bool flammable = false;
    bool onFire = false;
    int fuelTime = 10;
    bool corrosive = false;
    bool corrodable = true;
    bool isFreeFalling = false;
    int seed;
};

struct DebugParticleInfo {
    Particle particle;
    int x;
    int y;
};

class ParticleSystem : public sf::Drawable, public sf::Transformable
{
public:
    ParticleSystem(unsigned int count) : threadPool(16),
        screen_particles(512 * 512 * 8, sf::Vertex(sf::Vector2f(0, 0), sf::Color::Red)),
        pixels(512, std::vector<Particle>(512, Particle(ParticleType::air))),
        chunk_borders((512 * 512) / (64 * 64) * 8, sf::Vertex(sf::Vector2f(0, 0), sf::Color::Red)),
        maxX(pixels.size()),
        maxY(pixels[0].size()),
        frameIndex(0) {}

    void add(Particle particle, int x, int y);
    bool valid_position(int xCoord, int yCoord);
    void pause(bool pauseState);
    bool isPaused();
    void showChunkBorders(bool showBorders);
    void nextFrame();
    void update();
    DebugParticleInfo getParticleInfoFromPos(int xCoord, int yCoord);

private:
    void addParticleToDraw(int x, int y, sf::Color particle_color);
    void addChunkToDraw(int xSquare, int ySquare);
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;

private:
    ctpl::thread_pool threadPool;
    std::vector<std::vector<Particle>> pixels;
    std::vector<sf::Vertex> screen_particles;
    std::vector<sf::Vertex> chunk_borders;
    bool chunkBordersDraw = false;
    bool pauseState = false;
    bool goNextFrame = false;
    unsigned int maxX;
    unsigned int maxY;
    int frameIndex;
};