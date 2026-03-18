#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

class Slider {
public:
    Slider(const sf::Vector2f& size, const sf::Vector2f& position, float minVal, float maxVal, float initialVal, const sf::Font& font, const std::string& label);

    void draw(sf::RenderWindow& window);
    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    
    float getValue() const;
    void setValue(float val);
    void setPosition(const sf::Vector2f& position);
    void setOnValueChange(std::function<void(float)> callback);

private:
    void updateKnobPosition();

    sf::RectangleShape track;
    sf::RectangleShape knob;
    sf::Text labelText;
    sf::Text valueText;
    
    float minValue;
    float maxValue;
    float currentValue;
    
    bool isDragging;
    std::function<void(float)> onValueChange;
};
