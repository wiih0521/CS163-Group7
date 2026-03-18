#include "UI/Slider.h"
#include <iomanip>
#include <sstream>

Slider::Slider(const sf::Vector2f& size, const sf::Vector2f& position, float minVal, float maxVal, float initialVal, const sf::Font& font, const std::string& label)
    : minValue(minVal), maxValue(maxVal), currentValue(initialVal), isDragging(false)
{
    track.setSize(sf::Vector2f(size.x, 4.f));
    track.setPosition(position.x, position.y + size.y / 2.f);
    track.setFillColor(sf::Color(100, 100, 100));

    knob.setSize(sf::Vector2f(10.f, 20.f));
    knob.setOrigin(5.f, 10.f);
    knob.setFillColor(sf::Color(200, 200, 200));

    labelText.setFont(font);
    labelText.setString(label);
    labelText.setCharacterSize(14);
    labelText.setFillColor(sf::Color::White);
    labelText.setPosition(position.x, position.y - 18.f);

    valueText.setFont(font);
    valueText.setCharacterSize(14);
    valueText.setFillColor(sf::Color::White);

    updateKnobPosition();
}

void Slider::updateKnobPosition() {
    float ratio = (currentValue - minValue) / (maxValue - minValue);
    knob.setPosition(track.getPosition().x + ratio * track.getSize().x, track.getPosition().y + track.getSize().y / 2.f);
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(1) << currentValue << "s";
    valueText.setString(ss.str());
    sf::FloatRect valBounds = valueText.getLocalBounds();
    valueText.setPosition(track.getPosition().x + track.getSize().x - valBounds.width, track.getPosition().y - 24.f);
}

void Slider::draw(sf::RenderWindow& window) {
    window.draw(labelText);
    window.draw(track);
    window.draw(knob);
    window.draw(valueText);
}

bool Slider::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
            if (knob.getGlobalBounds().contains(mousePos) || track.getGlobalBounds().contains(mousePos)) {
                isDragging = true;
            }
        }
    }
    else if (event.type == sf::Event::MouseButtonReleased) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            isDragging = false;
        }
    }
    else if (event.type == sf::Event::MouseMoved && isDragging) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
        float localX = mousePos.x - track.getPosition().x;
        float ratio = std::max(0.f, std::min(1.f, localX / track.getSize().x));
        currentValue = minValue + ratio * (maxValue - minValue);
        updateKnobPosition();
        if (onValueChange) onValueChange(currentValue);
        return true;
    }
    return false;
}

float Slider::getValue() const { return currentValue; }
void Slider::setValue(float val) { currentValue = val; updateKnobPosition(); }
void Slider::setPosition(const sf::Vector2f& position) {
    float dx = position.x - track.getPosition().x;
    float dy = position.y - track.getPosition().y;
    track.move(dx, dy);
    knob.move(dx, dy);
    labelText.move(dx, dy);
    valueText.move(dx, dy);
}
void Slider::setOnValueChange(std::function<void(float)> callback) { onValueChange = callback; }
