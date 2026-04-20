#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>

struct Token {
    std::string text;
    sf::Color color;
};

class CodeHighlight {
private:
    float x_pos;
    float y_pos;
    float width;
    float height;
    sf::RectangleShape background;
    const sf::Font* titleFont;
    sf::Font codeFont;
    bool codeFontLoaded;
    std::vector<std::string> codeLines;
    std::vector<int> activeLines;
    std::vector<std::vector<Token>> cachedTokens;
    
    sf::Color getTokenColor(const std::string& token);

public:
    CodeHighlight();
    CodeHighlight(float x, float y, float w, float h, const sf::Font* f);

    void init(float x, float y, float w, float h, const sf::Font* f);
    void setCode(const std::vector<std::string>& lines);
    void setActiveLines(const std::vector<int>& lines);
    void onResize(float x, float y, float w, float h);

    void draw(sf::RenderWindow& window);
};
