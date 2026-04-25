#pragma once

#include <SFML/Graphics/Color.hpp>

namespace Theme {
enum class ThemeMode {
    DARK,
    LIGHT
};

extern ThemeMode currentTheme;

inline sf::Color withAlpha(sf::Color c, sf::Uint8 a) {
    return sf::Color(c.r, c.g, c.b, a);
}

namespace DarkTheme {
    inline sf::Color background() { return sf::Color(0x29, 0x2F, 0x36); }
    inline sf::Color surface() { return sf::Color(0xE0, 0xDB, 0xD8); }
    inline sf::Color surfaceAlt() { return sf::Color(0xFA, 0xF5, 0xF1); }
    inline sf::Color muted() { return sf::Color(0x8F, 0x7A, 0x6E); }
    inline sf::Color accent() { return sf::Color(0xA4, 0x1F, 0x13); }
    inline sf::Color textOnDark() { return surfaceAlt(); }
    inline sf::Color textOnLight() { return background(); }
}

namespace LightTheme {
    inline sf::Color background() { return sf::Color(0xFA, 0xF5, 0xF1); }
    inline sf::Color surface() { return sf::Color(0x29, 0x2F, 0x36); }
    inline sf::Color surfaceAlt() { return sf::Color(0x1F, 0x25, 0x2C); }
    inline sf::Color muted() { return sf::Color(0x70, 0x85, 0x91); }
    inline sf::Color accent() { return sf::Color(0xA4, 0x1F, 0x13); }
    inline sf::Color textOnDark() { return background(); }
    inline sf::Color textOnLight() { return surface(); }
}

inline sf::Color background() { 
    return currentTheme == ThemeMode::DARK ? DarkTheme::background() : LightTheme::background(); 
}

inline sf::Color surface() { 
    return currentTheme == ThemeMode::DARK ? DarkTheme::surface() : LightTheme::surface(); 
}

inline sf::Color surfaceAlt() { 
    return currentTheme == ThemeMode::DARK ? DarkTheme::surfaceAlt() : LightTheme::surfaceAlt(); 
}

inline sf::Color muted() { 
    return currentTheme == ThemeMode::DARK ? DarkTheme::muted() : LightTheme::muted(); 
}

inline sf::Color accent() { 
    return currentTheme == ThemeMode::DARK ? DarkTheme::accent() : LightTheme::accent(); 
}

inline sf::Color textOnDark() { 
    return currentTheme == ThemeMode::DARK ? DarkTheme::textOnDark() : LightTheme::textOnDark(); 
}

inline sf::Color textOnLight() { 
    return currentTheme == ThemeMode::DARK ? DarkTheme::textOnLight() : LightTheme::textOnLight(); 
}
}