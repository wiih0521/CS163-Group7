#include "DataStructure/LinkedList.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>

LinkedList::LinkedList(float windowWidth, float windowHeight) {
    winW = windowWidth;
    winH = windowHeight;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
        std::cerr << "Failed to load font\n";
    initUI();
}

LinkedList::~LinkedList() {}

void LinkedList::initUI() {
    buttons.clear();
    textInputs.clear();

    float uiY1 = winH - 160.f;
    float uiY2 = winH - 120.f;

    textInputs.push_back(TextInput(sf::Vector2f(120, 30), sf::Vector2f(300, uiY1), font, "e.g. 10,20,30"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(300, uiY2), "Init Array", font, [this]() {
        std::string t = textInputs[0].getText();
        std::vector<int> res; size_t pos = 0;
        while ((pos = t.find(',')) != std::string::npos) { if (pos > 0) res.push_back(std::stoi(t.substr(0, pos))); t.erase(0, pos + 1); }
        if (!t.empty()) res.push_back(std::stoi(t));
        if (!res.empty()) init(res);
    }));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(430, uiY2), "Init Random", font, [this]() {
        std::vector<int> r; for (int i = 0; i < 5; ++i) r.push_back(rand() % 100); init(r);
    }));
}
