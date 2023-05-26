#include "ParticleSystem.h"

Particle::Particle(ParticleType type, float density) {
    this->particleType = type;
    this->density = density;
    this->seed = rand()%10;
}

void Particle::setDensity(float density) {
    this->density = density;
}

void Particle::setLifeTime(int lifeTime) {
    this->lifeTime = lifeTime;
}

void Particle::setFlammability(bool flammable) {
    this->flammable = flammable;
}

void Particle::setFuelTime(int fuelTime) {
    this->fuelTime = fuelTime;
}

ParticleType Particle::getType() const {
    return this->particleType;
}

sf::Color Particle::getColor() const {
    sf::Color color;
    if (this->particleType == air){
        return sf::Color::Transparent;
    }
    switch (particleType)
    {
    case sand:
        color = sf::Color(209, 175, 23, 255);
        color.r += seed * 5;
        color.g += seed * 2;
        break;
    case solid:
        color = sf::Color::White;
        break;
    case gas:
        color = sf::Color(117, 171, 0, 155);
        color.g += (rand() % 40) - 20;
        break;
    case liquid:
        color = sf::Color::Blue;
        break;
    case fire:
        color = sf::Color::Red;
        break;
    default:
        color = sf::Color::Magenta;
        break;
    }
    if (this->onFire) {
        color.r = 255;
        color.g = color.g / 10;
    }
    return color;
};

void swapPixels(std::vector<std::vector<Particle>>& pixels, sf::Vector2u pixelCoords1, sf::Vector2u pixelCoords2) {
    Particle temp = pixels[pixelCoords1.x][pixelCoords1.y];
    pixels[pixelCoords1.x][pixelCoords1.y] = pixels[pixelCoords2.x][pixelCoords2.y];
    pixels[pixelCoords2.x][pixelCoords2.y] = temp;
}

bool Particle::validPosition(int boundsX, int boundsY, int xCoord, int yCoord) {
    return (xCoord >= 0 && xCoord < boundsX && yCoord >= 0 && yCoord < boundsY);
}

void Particle::updateSand(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY) {
    struct Offset {
        int x;
        int y;
    };
    Offset offsets[3] = { {0, 1}, {1, 1}, {-1, 1} };

    bool hasMoved = false;

    for (int i = 0; i < 1; i++) {
        Offset offset = offsets[i];
        if (!Particle::validPosition(boundsX, boundsY, x + offset.x, y + offset.y))
            return;

        if (pixels[x + offset.x][y + offset.y].particleType != ParticleType::air && pixels[x + offset.x][y + offset.y].hasBeenUpdated != hasBeenUpdated) {
            return;
        }

        if (pixels[x + offset.x][y + offset.y].particleType == ParticleType::air || pixels[x + offset.x][y + offset.y].density < this->density && pixels[x + offset.x][y + offset.y].particleType != solid) {
            swapPixels(pixels, sf::Vector2u(x, y), sf::Vector2u(x + offset.x, y + offset.y));
            this->isFreeFalling = true;
            return;
        }
    }
    for (int i = 1; i < 3; i++) {
        int randOffset = rand() % 2 + 1;
        Offset offset = offsets[randOffset];
        if (!Particle::validPosition(boundsX, boundsY, x + offset.x, y + offset.y))
            break;

        if ((pixels[x + offset.x][y + offset.y].particleType == ParticleType::air || pixels[x + offset.x][y + offset.y].density < this->density) && pixels[x + offset.x][y + offset.y].particleType != solid) {
            swapPixels(pixels, sf::Vector2u(x, y), sf::Vector2u(x + offset.x, y + offset.y));
            this->isFreeFalling = true;
            break;
        }
    }
}

void Particle::updateLiquid(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY) {
    struct Offset {
        int x;
        int y;
    };
    Offset offsets[5] = { {0, 1}, {1, 1}, {-1, 1}, {1, 0}, {-1, 0}, };
    for (int i = 0; i < 3; i++) {
        Offset offset = offsets[i];
        if (!Particle::validPosition(boundsX, boundsY, x + offset.x, y + offset.y))
            return;
        if ((pixels[x + offset.x][y + offset.y].particleType == ParticleType::air || pixels[x + offset.x][y + offset.y].density < this->density) && pixels[x + offset.x][y + offset.y].particleType != solid) {
            swapPixels(pixels, sf::Vector2u(x, y), sf::Vector2u(x + offset.x, y + offset.y));
            return;
        }
    }
    int maxOffset[2] = { 0, 0 };
    for (int i = 3; i < 5; i++) {
        Offset offset = offsets[i];
        int maxValidOffset = 0;
        for (int j = 1; j <= 4; j++) {
            if (!Particle::validPosition(boundsX, boundsY, x + offset.x*j, y))
                break;
            maxValidOffset = j*offset.x;
        }
        if (maxValidOffset < 0) maxValidOffset *= -1;
        for (int j = 1; j <= maxValidOffset; j++) {
            if (pixels[x + offset.x * j][y].particleType == air || pixels[x + offset.x * j][y].particleType == liquid && !(validPosition(boundsX, boundsY, x, y + 1) && pixels[x + maxOffset[i-3]][y + 1].particleType == air))
                maxOffset[i-3] = j * offset.x;
            else continue;
        }
    }
    int randIndex = rand() % 2;
    swapPixels(pixels, sf::Vector2u(x, y), sf::Vector2u(x + maxOffset[randIndex], y));
}

void Particle::updateGas(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY) {
    struct Offset {
        int x;
        int y;
    };
    Offset offsets[5] = { {0, -1}, {1, -1}, {-1, -1}, {1, 0}, {-1, 0}, };
    if (this->onFire) {
        if (this->fuelTime < 0) {
            this->onFire = false;
            this->particleType = air;
            return;
        }
        this->fuelTime -= (rand() % 3);

        fireSpread(pixels, x, y, boundsX, boundsY);
    }
    for (int i = 0; i < 5; i++) {
        Offset offset = offsets[i];
        if (!Particle::validPosition(boundsX, boundsY, x + offset.x, y + offset.y))
            return;
        if (pixels[x + offset.x][y + offset.y].particleType == ParticleType::air || (pixels[x + offset.x][y + offset.y].particleType == ParticleType::gas && pixels[x + offset.x][y + offset.y].density < density)) {
            swapPixels(pixels, sf::Vector2u(x, y), sf::Vector2u(x + offset.x, y + offset.y));
            break;
        }
    }
}

void Particle::fireSpread(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY) {
    struct Offset {
        int x;
        int y;
    };
    Offset offsets[4] = { {0, 1}, {1, 0}, {-1, 0} , {0, -1} };
    for (int i = 0; i < 4; i++) {
        Offset offset = offsets[i];
        if (!Particle::validPosition(boundsX, boundsY, x + offset.x, y + offset.y))
            continue;
        if (pixels[x + offset.x][y + offset.y].flammable && (rand() % 3 == 2)) 
            pixels[x + offset.x][y + offset.y].onFire = true;
    }
}

void Particle::updateFire(std::vector<std::vector<Particle>>& pixels, int x, int y, int boundsX, int boundsY) {
    struct Offset {
        int x;
        int y;
    };
    this->lifeTime -= (rand() % 2);
    if (this->lifeTime <= 0) {
        this->particleType = air;
        this->onFire = false;
        return;
    }
    fireSpread(pixels, x, y, boundsX, boundsY);
    Offset offsets[3] = { {0, 1}, {-1, 1}, {1, 1} };
    for (int i = 0; i < 3; i++) {
        Offset offset = offsets[i];
        if (!Particle::validPosition(boundsX, boundsY, x + offset.x, y + offset.y))
            continue;
        if (pixels[x + offset.x][y + offset.y].particleType == ParticleType::air || (pixels[x + offset.x][y + offset.y].particleType == ParticleType::gas && pixels[x + offset.x][y + offset.y].density < density)) {
            swapPixels(pixels, sf::Vector2u(x, y), sf::Vector2u(x + offset.x, y + offset.y));
            break;
        }
    }
}

void Particle::update(std::vector<std::vector<Particle>>& pixels, int x, int y) {
    this->hasBeenUpdated++;
    this->hasBeenUpdated %= 2;

    int boundsX = pixels[0].size();
    int boundsY = pixels.size();

    switch (this->particleType) {
    case sand:
        updateSand(pixels, x, y, boundsX, boundsY);
        break;
    case liquid:
        updateLiquid(pixels, x, y, boundsX, boundsY);
        break;
    case gas:
        updateGas(pixels, x, y, boundsX, boundsY);
        break;
    case fire:
        updateFire(pixels, x, y, boundsX, boundsY);
        break;
    default:
        break;
    }
}

bool Particle::shouldUpdate(int frame_index) {
    return (frame_index == hasBeenUpdated);
}

int Particle::getUpdateIndex()
{
    return this->hasBeenUpdated;
}

