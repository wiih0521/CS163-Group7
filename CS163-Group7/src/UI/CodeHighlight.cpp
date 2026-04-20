#include "UI/CodeHighlight.h"
#include <sstream>
#include <cctype>
#include <algorithm>

CodeHighlight::CodeHighlight() : titleFont(nullptr), codeFontLoaded(false) {}

CodeHighlight::CodeHighlight(float x, float y, float w, float h, const sf::Font* f) {
    init(x, y, w, h, f);
}

void CodeHighlight::init(float x, float y, float w, float h, const sf::Font* f) {
    x_pos = x;
    y_pos = y;
    width = w;
    height = h;
    titleFont = f;

    if (!codeFontLoaded) {
        if (codeFont.loadFromFile("assets/fonts/consola.ttf"))
            codeFontLoaded = true;
    }

    background.setSize(sf::Vector2f(w, h));
    background.setPosition(x, y);
    background.setFillColor(sf::Color(30, 30, 30)); 
    background.setOutlineThickness(1.f);
    background.setOutlineColor(sf::Color(64, 64, 64));
}

void CodeHighlight::onResize(float x, float y, float w, float h) {
    init(x, y, w, h, titleFont);
}

sf::Color CodeHighlight::getTokenColor(const std::string& token) {
    if (token.empty())
        return sf::Color::White;
    
    std::string lower = token;
    for (char& c : lower)
        c = std::tolower(c);

    if (lower == "if" || lower == "else" || lower == "while" || lower == "for" ||
        lower == "return" || lower == "int" || lower == "bool" || lower == "void" ||
        lower == "new" || lower == "delete" || lower == "struct" || lower == "class") {
        return sf::Color(86, 156, 214); 
    }

    if (lower == "true" || lower == "false" || lower == "null" || lower == "nullptr")
        return sf::Color(86, 156, 214);

    bool isNumber = true;
    for (char c : token) {
        if (!std::isdigit(c)) {
            isNumber = false;
            break;
        }
    }
    
    if (isNumber)
        return sf::Color(181, 206, 168);

    if (token == "insert" || token == "remove" || token == "play" || token == "search")
        return sf::Color(220, 220, 170); 

    return sf::Color(212, 212, 212);
}

void CodeHighlight::setCode(const std::vector<std::string>& lines) {
    codeLines = lines;
    cachedTokens.clear();
    
    for (const auto& line : lines) {
        std::vector<Token> lineTokens;
        std::string currentToken = "";
        
        for (size_t i = 0; i < line.size(); ++i) {
            char c = line[i];

            if (std::isspace(c) || c == '(' || c == ')' || c == '{' || c == '}' || 
                c == ';' || c == ',' || c == '=' || c == '<' || c == '>' || 
                c == '!' || c == '+' || c == '-') {
                
                if (!currentToken.empty()) {
                    lineTokens.push_back({currentToken, getTokenColor(currentToken)});
                    currentToken.clear();
                }
                
                std::string p(1, c);
                sf::Color pColor = sf::Color(212, 212, 212);

                if (c == '=' || c == '<' || c == '>' || c == '!' || c == '+' || c == '-')
                    pColor = sf::Color(212, 212, 212); 
                
                lineTokens.push_back({p, pColor});
            } else {
                currentToken += c;
            }
        }
        
        if (!currentToken.empty())
            lineTokens.push_back({currentToken, getTokenColor(currentToken)});
        
        cachedTokens.push_back(lineTokens);
    }
}

void CodeHighlight::setActiveLines(const std::vector<int>& lines) {
    activeLines = lines;
}

void CodeHighlight::draw(sf::RenderWindow& window) {
    if (!titleFont && !codeFontLoaded)
        return;

    window.draw(background);

    float startX = x_pos + 15.f;
    float startY = y_pos + 50.f; 
    float lineHeight = 24.f;

    sf::Text title("Code Highlight", titleFont ? *titleFont : codeFont, 18);
    title.setPosition(startX, y_pos + 15.f);
    title.setFillColor(sf::Color(220, 220, 220));
    window.draw(title);

    const sf::Font& renderFont = codeFontLoaded ? codeFont : (titleFont ? *titleFont : codeFont);

    for (size_t i = 0; i < codeLines.size(); ++i) {
        float currentY = startY + i * lineHeight;
        
        if (std::find(activeLines.begin(), activeLines.end(), i) != activeLines.end()) {
            sf::RectangleShape hl(sf::Vector2f(width, lineHeight));
            hl.setPosition(x_pos, currentY);
            hl.setFillColor(sf::Color(60, 60, 60, 150)); 
            window.draw(hl);
        }

        if (i < cachedTokens.size()) {
            float currentX = startX;

            for (const auto& token : cachedTokens[i]) {
                sf::Text textHolder(sf::String::fromUtf8(token.text.begin(), token.text.end()), renderFont, 15);
                textHolder.setFillColor(token.color);
                textHolder.setPosition(currentX, currentY);
                window.draw(textHolder);

                currentX += textHolder.getLocalBounds().width;
            }
        }
    }
}
