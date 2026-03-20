#pragma once
#include "DataStructure.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <string>
#include "UI/Button.h"
#include "UI/TextInput.h"

class MinMaxHeap : public DataStructure {
public:
    MinMaxHeap(float windowWidth = 1280.f, float windowHeight = 720.f);
    ~MinMaxHeap() override;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;
    void play() override;
    void pause() override;
    void stepForward() override;
    void stepBackward() override;
    void onResize(float w, float h) override;

    void init(const std::vector<int>& data);
    void insert(int value);
    void extract();
    void buildHeap();

private:
    struct Node {
        int value;
        sf::Vector2f position;
        sf::Vector2f targetPosition;
    };
    std::vector<int> rawData;
    std::vector<Node> nodes;
    bool isMinHeap = true;

    sf::Font font;
    std::vector<Button> buttons;
    std::vector<TextInput> textInputs;
    void initUI();
    void recalculateTargetPositions();
    void heapifyUp(int index);
    void heapifyDown(int index);

    struct VisualStep {
        std::vector<int> highlighted;
        std::string message;
        sf::Color highlightColor  = sf::Color(220, 180, 0);
        sf::Color highlightColor2 = sf::Color(100, 180, 255);
    };
    std::vector<VisualStep> animSteps;
    int animStep = -1;
    bool isPlaying = false;
    float playTimer = 0.f;
    std::function<void()> commitOp;

    void beginInsertSteps(int value);
    void beginExtractSteps();
    void beginSearchSteps(int value);
};