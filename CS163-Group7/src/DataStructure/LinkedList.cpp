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
    textInputs.push_back(TextInput(sf::Vector2f(60, 30), sf::Vector2f(560, uiY1), font, "Value"));
    textInputs.push_back(TextInput(sf::Vector2f(50, 30), sf::Vector2f(630, uiY1), font, "Idx"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(560, uiY2), "Add Node", font, [this]() {
        std::string v = textInputs[1].getText(), i = textInputs[2].getText();
        if (!v.empty()) beginInsertSteps(std::stoi(v), i.empty() ? (int)nodes.size() : std::stoi(i));
    }));
    buttons.push_back(Button(sf::Vector2f(110, 30), sf::Vector2f(690, uiY2), "Add Random", font, [this]() {
        beginInsertSteps(rand() % 100, (int)nodes.size());
    }));
    textInputs.push_back(TextInput(sf::Vector2f(120, 30), sf::Vector2f(810, uiY1), font, "Index to Del"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(810, uiY2), "Delete Node", font, [this]() {
        std::string i = textInputs[3].getText();
        if (!i.empty()) beginDeleteSteps(std::stoi(i));
    }));
    textInputs.push_back(TextInput(sf::Vector2f(90, 30), sf::Vector2f(940, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90, 30), sf::Vector2f(940, uiY2), "Search", font, [this]() {
        std::string v = textInputs[4].getText();
        if (!v.empty()) beginSearchSteps(std::stoi(v));
    }));
}

void LinkedList::init(const std::vector<int>& data) {
    nodes.clear();
    float totalWidth = data.size() * 100.f;
    float startX = 250.f + ((winW - 250.f - totalWidth) / 2.f);
    for (size_t i = 0; i < data.size(); ++i) {
        Node n; n.value = data[i];
        n.targetPosition = sf::Vector2f(startX + i*100.f, winH / 2.f - 50.f);
        n.position = sf::Vector2f(startX + i*100.f, -50.f);
        nodes.push_back(n);
    }
}

void LinkedList::insert(int value, int index) {
    if (index < 0) index = 0;
    if (index > (int)nodes.size()) index = (int)nodes.size();
    Node n; n.value = value;
    n.position = sf::Vector2f(250.f + (winW - 250.f) / 2.f, winH + 100.f);
    nodes.insert(nodes.begin() + index, n);
    float totalWidth = nodes.size() * 100.f;
    float startX = 250.f + ((winW - 250.f - totalWidth) / 2.f);
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
    }
}

void LinkedList::remove(int index) {
    if (index < 0 || index >= (int)nodes.size()) return;
    nodes.erase(nodes.begin() + index);
    float totalWidth = nodes.size() * 100.f;
    float startX = 250.f + ((winW - 250.f - totalWidth) / 2.f);
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
    }
}
