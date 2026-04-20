#include "UI/Button.h"
#include <cmath>

static const float PI = 3.14159265f;

static sf::VertexArray makeRoundedRect(sf::Vector2f pos, sf::Vector2f size, float radius, sf::Color color, int segments = 6) {
    if (radius > size.x / 2.f) radius = size.x / 2.f;
    if (radius > size.y / 2.f) radius = size.y / 2.f;

    sf::Vector2f corners[4] = {
        { pos.x + radius,          pos.y + radius },        
        { pos.x + size.x - radius, pos.y + radius },         
        { pos.x + size.x - radius, pos.y + size.y - radius }, 
        { pos.x + radius,          pos.y + size.y - radius }  
    };
    float startAngles[4] = { PI, 3.f*PI/2.f, 0.f, PI/2.f };

    sf::Vector2f center = { pos.x + size.x / 2.f, pos.y + size.y / 2.f };

    sf::VertexArray va(sf::TriangleFan);
    va.append({ center, color });

    for (int c = 0; c < 4; ++c) {
        for (int s = 0; s <= segments; ++s) {
            float angle = startAngles[c] + (PI / 2.f) * s / segments;
            sf::Vector2f p = { corners[c].x + radius * std::cos(angle),
                               corners[c].y + radius * std::sin(angle) };
            va.append({ p, color });
        }
    }

    float angle0 = startAngles[0];
    sf::Vector2f first = { corners[0].x + radius * std::cos(angle0),
                           corners[0].y + radius * std::sin(angle0) };
    va.append({ first, color });
    return va;
}

Button::Button(const sf::Vector2f& size, const sf::Vector2f& position, const std::string& text, const sf::Font& font, std::function<void()> onClick)
    : onClick(onClick), isHovered(false), isPressed(false), isActive(false) {
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
    sf::Color fillColor;
    if (!isEnabled) {
        fillColor = sf::Color(40, 40, 40);
        buttonText.setFillColor(sf::Color(100, 100, 100));
    } else {
        buttonText.setFillColor(sf::Color::White);
        if (isActive)        fillColor = activeColor;
        else if (isPressed)  fillColor = pressedColor;
        else if (isHovered)  fillColor = hoverColor;
        else                 fillColor = normalColor;
    }

    const float radius = 8.f;
    sf::Vector2f pos  = shape.getPosition();
    sf::Vector2f size = shape.getSize();

    auto body = makeRoundedRect(pos, size, radius, fillColor);
    window.draw(body);

    sf::Color outlineColor = isEnabled ? sf::Color(180, 180, 180, 120) : sf::Color(80, 80, 80, 120);
    auto outline = makeRoundedRect(pos, size, radius, outlineColor, 8);

    sf::VertexArray border(sf::LineStrip, outline.getVertexCount() - 1);
    for (std::size_t i = 1; i < outline.getVertexCount(); ++i)
        border[i - 1] = { outline[i].position, outlineColor };
    window.draw(border);

    window.draw(buttonText);
}

void Button::setActive(bool active) {
    isActive = active;
}

void Button::setEnabled(bool enabled) {
    isEnabled = enabled;
}

bool Button::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
    sf::Vector2f mousePosF((float)mousePos.x, (float)mousePos.y);

    bool contains = shape.getGlobalBounds().contains(mousePosF);
    
    if (event.type == sf::Event::MouseMoved)
        isHovered = contains;
    
    if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
        if (contains && isEnabled) {
            isPressed = true;
            return true;
        }
    }
    
    if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left) {
        if (isPressed && contains) {
            isPressed = false;
            if (onClick) {
                auto cb = onClick;
                cb();
                return true;
            }
        }

        isPressed = false;
    }
    
    return false;
}

void Button::setPosition(const sf::Vector2f& position) {
    shape.setPosition(position);
    buttonText.setPosition(position.x + shape.getSize().x / 2.0f, position.y + shape.getSize().y / 2.0f);
}

void Button::setText(const std::string& text) {
    buttonText.setString(text);
    sf::FloatRect textBounds = buttonText.getLocalBounds();
    buttonText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    buttonText.setPosition(shape.getPosition().x + shape.getSize().x / 2.0f, shape.getPosition().y + shape.getSize().y / 2.0f);
}
