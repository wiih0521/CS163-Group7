#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>
#include "DataStructure/DataStructure.h"
#include "UI/Button.h"
#include "UI/Slider.h"

class App {
public:
    App();
    ~App();
    void run();

private:
    void processEvents();
    void update(float dt);
    void render();
    void initUI();

    sf::RenderWindow window;
    std::unique_ptr<DataStructure> currentDS;
    sf::Clock deltaClock;
    bool uiNeedsUpdate = false;
    bool isStepMode = true;
    bool showExitConfirm = false;
    
    float windowWidth;
    float windowHeight;
    
    sf::Font font;
    
    enum class Category {
        LINKED_LIST,
        HEAP,
        TREE,
        GRAPH
    };
    Category activeCategory;
    
    enum class DSType {
        NONE,
        SINGLY_LINKED_LIST,
        MIN_MAX_HEAP,
        AVL_TREE,
        KRUSKAL_MST,
        PRIM_MST
    };
    DSType activeDS;

    std::vector<Button> categoryButtons;
    std::vector<Button> dsButtons;
    std::vector<Button> controlButtons;
    std::vector<Button> exitConfirmButtons;
    std::unique_ptr<Slider> speedSlider;
    
    void buildCategoryButtons();
    void buildDSButtons();
    
    sf::RectangleShape sidebar;
};
