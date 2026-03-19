#pragma once
#include <SFML/Graphics.hpp>

class DataStructure {
protected:
    bool isStepByStep = true;
    float winW = 1280.f;
    float winH = 720.f;
    float playInterval = 0.6f;
public:
    virtual ~DataStructure() = default;
    virtual void update(float dt) = 0;
    virtual void draw(sf::RenderWindow& window) = 0;
    virtual void handleEvent(const sf::Event& event, const sf::RenderWindow& window) = 0;
    virtual void play() = 0;
    virtual void pause() = 0;
    virtual void stepForward() = 0;
    virtual void stepBackward() = 0;
    virtual void setStepMode(bool step) { isStepByStep = step; }
    virtual void setPlayInterval(float interval) { playInterval = interval; }
    virtual void onResize(float w, float h) { winW = w; winH = h; }
};