#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Theme {
inline sf::Color withAlpha(sf::Color c, sf::Uint8 a) {
    return sf::Color(c.r, c.g, c.b, a);
}

// Use functions (not inline variables) for C++11/14 compatibility.
inline sf::Color background() { return sf::Color(0x29, 0x2F, 0x36); }
inline sf::Color surface() { return sf::Color(0xE0, 0xDB, 0xD8); }
inline sf::Color surfaceAlt() { return sf::Color(0xFA, 0xF5, 0xF1); }
inline sf::Color muted() { return sf::Color(0x8F, 0x7A, 0x6E); }
inline sf::Color accent() { return sf::Color(0xA4, 0x1F, 0x13); }

inline sf::Color textOnDark() { return surfaceAlt(); }
inline sf::Color textOnLight() { return background(); }
} // namespace Theme

