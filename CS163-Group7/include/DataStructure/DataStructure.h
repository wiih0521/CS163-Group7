#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

class DataStructure {
protected:
    bool isStepByStep = true;
    float winW = 1280.f;
    float winH = 720.f;
    float codePaneWidth = 0.f;
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
    virtual void initUI() = 0;
    virtual void onResize(float w, float h) { winW = w; winH = h; codePaneWidth = w / 6.0f; initUI(); }

    // Code display support
    virtual std::vector<std::string> getCode() const { return {}; }
    virtual int getCurrentLine() const { return -1; }
};
