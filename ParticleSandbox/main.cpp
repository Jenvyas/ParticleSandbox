#include <SFML/Graphics.hpp>
#include "ParticleSystem.h"
#include "Menu.h"
#include <iostream>

enum mouseButton {
    right,
    left,
    none
};
struct mouseAction {
    bool mouseHeld = false;
    mouseButton mouseButton = none;
    ParticleType leftElem = sand;
    ParticleType rightElem = solid;
    int xCoord;
    int yCoord;
};

struct Brush {
    ParticleType activeElement;
    int brushSize;
};

namespace ParticleUtils {
    Particle newSandParticle() {
        Particle particle = Particle(ParticleType::sand);
        particle.setDensity(3.f);
        return particle;
    }
    Particle newWaterParticle() {
        Particle particle = Particle(ParticleType::liquid);
        particle.setDensity(1.f);
        return particle;
    }
    Particle newGasParticle() {
        Particle particle = Particle(ParticleType::gas);
        particle.setDensity(1.f);
        particle.setFlammability(true);
        particle.setFuelTime(16);
        return particle;
    }
    Particle newWallParticle() {
        Particle particle = Particle(ParticleType::solid);
        return particle;
    }
    Particle newFireParticle() {
        Particle particle = Particle(ParticleType::fire);
        particle.setLifeTime(10);
        return particle;
    }
    Particle newAirParticle() {
        Particle particle = Particle(ParticleType::air);
        return particle;
    }

    Particle newParticle(ParticleType particleType) {
        switch (particleType) {
        case sand:
            return newSandParticle();
            break;
        case liquid:
            return newWaterParticle();
            break;
        case gas:
            return newGasParticle();
            break;
        case solid:
            return newWallParticle();
            break;
        case fire:
            return newFireParticle();
            break;
        case air:
            return newAirParticle();
            break;
        }
    }
}

void updateDebugText(sf::Text& debugInfo, DebugParticleInfo& debugParticle) {
    debugInfo.setFillColor(debugParticle.particle.getColor());
    debugInfo.setOutlineColor(sf::Color::Red);
    std::string debugText = std::to_string(debugParticle.x) + "; " + std::to_string(debugParticle.x) + ", " + std::to_string(debugParticle.particle.getUpdateIndex());
    debugInfo.setString(debugText);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(512, 512), "SFML works!");
    sf::Font font;
    font.loadFromFile("arial.ttf");
    sf::Text debugInfo;
    debugInfo.setFont(font);
    bool debug = false;
    window.setFramerateLimit(60);
    Menu menu;
    int windowWidth = 512;
    int windowHeight = 512;
    int xCoord;
    int yCoord;
    ParticleSystem sandbox(0);
    DebugParticleInfo debugParticle = sandbox.getParticleInfoFromPos(-1, -1);
    if (debug) {
        debugInfo.setCharacterSize(20);
        updateDebugText(debugInfo, debugParticle);
        sandbox.showChunkBorders(true);
    }
    Brush brush;
    brush.activeElement = sand;
    brush.brushSize = 10;
    mouseAction mouseAction;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event))
        {
            switch (event.type)
            {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::Resized:
                if (event.type == sf::Event::Resized)
                    window.setSize(sf::Vector2u(event.size.width, event.size.height));
                windowWidth = event.size.width;
                windowHeight = event.size.height;
                break;
            case sf::Event::MouseButtonPressed:
                xCoord = ((float)event.mouseButton.x / windowWidth) * 512;
                yCoord = ((float)event.mouseButton.y / windowHeight) * 512;
                if (xCoord + brush.brushSize / 2 < 512 && yCoord + brush.brushSize / 2 < 512 &&
                    xCoord - brush.brushSize / 2 >= 0 && yCoord - brush.brushSize / 2 >= 0)
                {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        mouseAction.mouseHeld = true;
                        mouseAction.mouseButton = mouseButton::left;
                        for (int y = yCoord + brush.brushSize / 2; y >= yCoord - brush.brushSize / 2; y--)
                            for (int x = xCoord + brush.brushSize / 2; x >= xCoord - brush.brushSize / 2; x--)
                                if (brush.activeElement == ParticleType::gas || brush.activeElement == liquid || brush.activeElement == sand) {
                                    if (rand() % 2) {
                                        sandbox.add(ParticleUtils::newParticle(brush.activeElement), x, y);
                                    }
                                } else {
                                    sandbox.add(ParticleUtils::newParticle(brush.activeElement), x, y);
                                }
                    }
                }
                break;
            case sf::Event::MouseMoved:
                xCoord = ((float)event.mouseMove.x / windowWidth) * 512;
                yCoord = ((float)event.mouseMove.y / windowHeight) * 512;
                if (debug) {
                    debugParticle = sandbox.getParticleInfoFromPos(xCoord, yCoord);
                    updateDebugText(debugInfo, debugParticle);
                }
                if (mouseAction.mouseHeld)
                {
                    if (mouseAction.mouseButton == mouseButton::left) {
                        for (int y = yCoord + brush.brushSize / 2; y >= yCoord - brush.brushSize / 2; y--)
                            for (int x = xCoord + brush.brushSize / 2; x >= xCoord - brush.brushSize / 2; x--) {
                                if (brush.activeElement == ParticleType::gas || brush.activeElement == liquid || brush.activeElement == sand) {
                                    if (rand() % 2) {
                                        sandbox.add(ParticleUtils::newParticle(brush.activeElement), x, y);
                                    }
                                } else {
                                    sandbox.add(ParticleUtils::newParticle(brush.activeElement), x, y);
                                }
                            }        
                    }
                }
                break;
            case sf::Event::MouseButtonReleased:
                mouseAction.mouseHeld = false;
                break;
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::Num1) {
                    brush.activeElement = sand;
                }
                if (event.key.code == sf::Keyboard::Num2) {
                    brush.activeElement = liquid;
                }
                if (event.key.code == sf::Keyboard::Num3) {
                    brush.activeElement = gas;
                }
                if (event.key.code == sf::Keyboard::Num4) {
                    brush.activeElement = fire;
                }
                if (event.key.code == sf::Keyboard::Num0) {
                    brush.activeElement = air;
                }
                if (event.key.code == sf::Keyboard::W) {
                    brush.activeElement = solid;
                }
                if (event.key.code == sf::Keyboard::D) {
                    debugInfo.setCharacterSize(20);
                    updateDebugText(debugInfo, debugParticle);
                    sandbox.showChunkBorders(true);
                    debug = true;
                }
                if (event.key.code == sf::Keyboard::Space) {
                    sandbox.pause(!sandbox.isPaused());
                }
                if (event.key.code == sf::Keyboard::Right) {
                    sandbox.nextFrame();
                }
                break;
            }
        }
        window.setActive(true);

        window.clear(sf::Color::Black); //sf::Color(236, 64, 103)
        sandbox.update();
        window.draw(sandbox);
        window.draw(menu);
        if (debug) {
            window.draw(debugInfo);
        }
        window.display();
    }
}