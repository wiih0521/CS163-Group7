#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <functional>

class TextInput {
public:
    TextInput(const sf::Vector2f& size, const sf::Vector2f& position, const sf::Font& font, const std::string& placeholder = "");

    void draw(sf::RenderWindow& window);
    bool handleEvent(const sf::Event& event, const sf::RenderWindow& window);
    
    void setPosition(const sf::Vector2f& position);
    std::string getText() const;
    void setText(const std::string& text);
    
private:
    sf::RectangleShape shape;
    sf::Text textDisplay;
    sf::Text placeholderText;
    
    std::string inputString;
    bool isFocused;
    bool isHovered;
    
    sf::Color normalColor;
    sf::Color hoverColor;
    sf::Color focusedColor;
    sf::Color textColor;
    sf::Color placeholderColor;
};
