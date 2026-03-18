#include "UI/Button.h"

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position, const std::string& text, const sf::Font& font, std::function<void()> onClick)
    : onClick(onClick), isHovered(false), isPressed(false), isActive(false)
{
    normalColor = sf::Color(60, 60, 60);
    hoverColor = sf::Color(100, 100, 100);
    pressedColor = sf::Color(40, 40, 40);
    activeColor = sf::Color(0, 120, 215); 

    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(normalColor);
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(150, 150, 150));

    buttonText.setFont(font);
    buttonText.setString(text);
    buttonText.setCharacterSize(16);
    buttonText.setFillColor(sf::Color::White);
    
    sf::FloatRect textBounds = buttonText.getLocalBounds();
    buttonText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    buttonText.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::draw(sf::RenderWindow& window) {
    if (isActive) shape.setFillColor(activeColor);
    else if (isPressed) shape.setFillColor(pressedColor);
    else if (isHovered) shape.setFillColor(hoverColor);
    else shape.setFillColor(normalColor);
    
    window.draw(shape);
    window.draw(buttonText);
}

void Button::setActive(bool active) {
    isActive = active;
}

bool Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF((float)mousePos.x, (float)mousePos.y);

    bool contains = shape.getGlobalBounds().contains(mousePosF);
    
    if (event.type == sf::Event::MouseMoved) {
        isHovered = contains;
    }
    
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (contains) {
            isPressed = true;
            return true;
        }
    }
    
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (isPressed && contains) {
            if (onClick) onClick();
        }
        isPressed = false;
    }
    
    return false;
}

void Button::setPosition(const sf::Vector2f& position) {
    shape.setPosition(position);
    sf::FloatRect textBounds = buttonText.getLocalBounds();
    buttonText.setPosition(position.x + shape.getSize().x / 2.0f, position.y + shape.getSize().y / 2.0f);
}

void Button::setText(const std::string& text) {
    buttonText.setString(text);
    sf::FloatRect textBounds = buttonText.getLocalBounds();
    buttonText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    buttonText.setPosition(shape.getPosition().x + shape.getSize().x / 2.0f, shape.getPosition().y + shape.getSize().y / 2.0f);
}
