#pragma once
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

class Button {
public:
    Button(const sf::Vector2f& size, const sf::Vector2f& position, const std::string& text, const sf::Font& font, std::function<void()> onClick);

    void draw(sf::RenderWindow& window);
    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    void setPosition(const sf::Vector2f& position);
    void setText(const std::string& text);
    void setActive(bool active);
    void setEnabled(bool enabled);
    
private:
    sf::RectangleShape shape;
    sf::Text buttonText;
    std::function<void()> onClick;
    bool isHovered;
    bool isPressed;
    bool isActive;
    bool isEnabled = true;
};
