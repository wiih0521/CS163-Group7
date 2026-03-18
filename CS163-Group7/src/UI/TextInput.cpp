#include "UI/TextInput.h"

TextInput::TextInput(const sf::Vector2f& size, const sf::Vector2f& position, const sf::Font& font, const std::string& placeholder)
    : isFocused(false), isHovered(false)
{
    normalColor = sf::Color(30, 30, 30);
    hoverColor = sf::Color(50, 50, 50);
    focusedColor = sf::Color(40, 40, 40);
    textColor = sf::Color::White;
    placeholderColor = sf::Color(120, 120, 120);

    shape.setSize(size);
    shape.setPosition(position);
    shape.setFillColor(normalColor);
    shape.setOutlineThickness(1.f);
    shape.setOutlineColor(sf::Color(100, 100, 100));

    textDisplay.setFont(font);
    textDisplay.setCharacterSize(16);
    textDisplay.setFillColor(textColor);
    textDisplay.setPosition(position.x + 5.f, position.y + size.y / 2.0f - 10.f);

    placeholderText.setFont(font);
    placeholderText.setString(placeholder);
    placeholderText.setCharacterSize(16);
    placeholderText.setFillColor(placeholderColor);
    placeholderText.setPosition(position.x + 5.f, position.y + size.y / 2.0f - 10.f);
}

void TextInput::draw(sf::RenderWindow& window) {
    if (isFocused) {
        shape.setFillColor(focusedColor);
        shape.setOutlineColor(sf::Color(0, 120, 215));
    } else if (isHovered) {
        shape.setFillColor(hoverColor);
        shape.setOutlineColor(sf::Color(150, 150, 150));
    } else {
        shape.setFillColor(normalColor);
        shape.setOutlineColor(sf::Color(100, 100, 100));
    }
    
    window.draw(shape);
    
    if (inputString.empty() && !isFocused) {
        window.draw(placeholderText);
    } else {
        std::string displayStr = inputString;
        if (isFocused) displayStr += "_";
        textDisplay.setString(displayStr);
        window.draw(textDisplay);
    }
}

bool TextInput::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF((float)mousePos.x, (float)mousePos.y);

    bool contains = shape.getGlobalBounds().contains(mousePosF);
    
    if (event.type == sf::Event::MouseMoved) {
        isHovered = contains;
    }
    
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        isFocused = contains;
    }
    
    if (isFocused && event.type == sf::Event::TextEntered) {
        if (event.text.unicode == 8 && !inputString.empty()) {
            inputString.pop_back();
            return true;
        }

        else if (event.text.unicode >= 32 && event.text.unicode < 128) {
            inputString += static_cast<char>(event.text.unicode);
            return true;
        }
    }
    
    return false;
}

void TextInput::setPosition(const sf::Vector2f& position) {
    shape.setPosition(position);
    textDisplay.setPosition(position.x + 5.f, position.y + shape.getSize().y / 2.0f - 10.f);
    placeholderText.setPosition(position.x + 5.f, position.y + shape.getSize().y / 2.0f - 10.f);
}

std::string TextInput::getText() const {
    return inputString;
}

void TextInput::setText(const std::string& text) {
    inputString = text;
}
