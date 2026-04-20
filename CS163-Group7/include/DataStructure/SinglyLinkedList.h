#pragma once
#include "DataStructure.h"
#include <vector>
#include <functional>
#include <memory>
#include <string>
#include "UI/Button.h"
#include "UI/TextInput.h"
#include "UI/InputDialog.h"

class SinglyLinkedList : public DataStructure {
public:
    SinglyLinkedList(float windowWidth = 1280.f, float windowHeight = 720.f);
    ~SinglyLinkedList() override;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;
    void play() override;
    void pause() override;
    void stepForward() override;
    void stepBackward() override;
    void onResize(float w, float h) override;

    void init(const std::vector<int>& data);
    void insert(int value, int index);
    void remove(int index);
    void search(int value);
    void updateNode(int index, int newValue);

private:
    struct Node {
        int value;
        sf::Vector2f position;
        sf::Vector2f targetPosition;
    };
    std::vector<Node> nodes;

    sf::Font font;
    std::vector<Button> buttons;
    std::vector<TextInput> textInputs;
    std::unique_ptr<InputDialog> activeDialog;
    void initUI();

    struct VisualStep {
        std::vector<int> highlighted;
        std::string message;
        sf::Color highlightColor = sf::Color(220, 180, 0); 
        std::vector<int> highlightedCodeLines;
    };
    std::vector<VisualStep> animSteps;
    int animStep = -1;
    bool isPlaying = false;
    float playTimer = 0.f;
    std::function<void()> commitOp;

    void beginInsertSteps(int value, int idx);
    void beginDeleteSteps(int idx);
    void beginSearchSteps(int value);
    void beginUpdateSteps(int idx, int newValue);
};
