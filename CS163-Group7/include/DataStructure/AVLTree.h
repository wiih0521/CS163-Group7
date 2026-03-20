#pragma once
#include "DataStructure.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <functional>
#include <string>
#include <map>
#include <set>
#include "UI/Button.h"
#include "UI/TextInput.h"

class AVLTree : public DataStructure {
public:
    AVLTree(float windowWidth = 1280.f, float windowHeight = 720.f);
    ~AVLTree() override;
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
    void remove(int value);

private:
    struct SimNode {
        int value, height;
        SimNode *left, *right;
    };

    struct TreeNode {
        int value, height;
        TreeNode* left;
        TreeNode* right;
        sf::Vector2f position;
        sf::Vector2f targetPosition;
        TreeNode(int v) : value(v), height(1), left(nullptr), right(nullptr) {}
    };
    TreeNode* root = nullptr;

    sf::Font font;
    std::vector<Button> buttons;
    std::vector<TextInput> textInputs;
    void initUI();

    int getHeight(TreeNode* n);
    int getBalance(TreeNode* n);
    TreeNode* rotateRight(TreeNode* y);
    TreeNode* rotateLeft(TreeNode* x);
    TreeNode* insertNode(TreeNode* node, int value, sf::Vector2f startPos = sf::Vector2f(600.f, 50.f));
    TreeNode* removeNode(TreeNode* node, int value);
    TreeNode* minValueNode(TreeNode* node);
    void deleteTree(TreeNode* node);
    void calcPositions(TreeNode* node, int depth, float hs, float vs, float startX, int& index);
    void drawNode(sf::RenderWindow& window, TreeNode* node,
                  const std::vector<int>& hlValues, sf::Color hlColor,
                  int pivotValue = -1, int unbalancedValue = -1);

    // Step-by-step animation
    struct VisualStep {
        std::vector<int> highlightedValues;
        std::string message;
        sf::Color highlightColor = sf::Color(220, 180, 0);
        int pivotValue = -1;       // Node about which we rotate
        int unbalancedValue = -1;  // Node that became unbalanced
        SimNode* treeSnapshot = nullptr; // Captured state of the tree
    };
    std::vector<VisualStep> animSteps;
    std::map<int, TreeNode*> nodeCache;

    // Internal simulation helpers
    SimNode* copySimTree(SimNode* node);
    void deleteSimTree(SimNode* node);
    int getSimHeight(SimNode* n);
    int getSimBalance(SimNode* n);
    SimNode* rotateRightSim(SimNode* y);
    SimNode* rotateLeftSim(SimNode* x);
    void insertNodeSim(SimNode** nodeRef, int value, std::vector<VisualStep>& steps, SimNode** rootRef);
    void removeNodeSim(SimNode** nodeRef, int value, std::vector<VisualStep>& steps, SimNode** rootRef);
    SimNode* minValueNodeSim(SimNode* node);

    float scaleFactor = 1.0f;
    void applySimStructure(SimNode* simRoot);
    void clearAnimSteps();
    int animStep = -1;
    bool isPlaying = false;
    float playTimer = 0.f;
    std::function<void()> commitOp;

    void beginInsertSteps(int value);
    void beginDeleteSteps(int value);
    void beginSearchSteps(int value);
};