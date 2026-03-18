#include "App.h"
#include "DataStructure/SinglyLinkedList.h"
#include "DataStructure/MinMaxHeap.h"
#include "DataStructure/AVLTree.h"
#include "DataStructure/GraphMST.h"
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

App::App() : window(sf::VideoMode(1280, 720), "Data Structure Visualizer", sf::Style::Default) {
#ifdef _WIN32
    HWND hwnd = window.getSystemHandle();
    ShowWindow(hwnd, SW_MAXIMIZE);
#endif

    windowWidth = (float)window.getSize().x;
    windowHeight = (float)window.getSize().y;
    window.setFramerateLimit(60);
    window.setMouseCursorVisible(true);
    
    // Initialize application state
    activeCategory = Category::LINKED_LIST;
    activeDS = DSType::SINGLY_LINKED_LIST;
    currentDS = std::make_unique<SinglyLinkedList>(windowWidth, windowHeight);
    isStepMode = true; // or whatever the default should be
    
    initUI();
}

App::~App() {
}

void App::initUI() {
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Error loading font for App UI\n";
    }

    // Sidebar
    sidebar.setSize(sf::Vector2f(250.f, windowHeight));
    sidebar.setPosition(0.f, 0.f);
    sidebar.setFillColor(sf::Color(40, 40, 40));

    // Clear UI vectors to prevent duplication on resize
    controlButtons.clear();
    exitConfirmButtons.clear();

    buildCategoryButtons();
    buildDSButtons();

    // Common controls
    float controlsY = windowHeight - 160.f;
    controlButtons.push_back(Button(sf::Vector2f(70, 35), sf::Vector2f(10, controlsY), "Play", font, [this]() {
        if (currentDS) currentDS->play();
    }));
    controlButtons.push_back(Button(sf::Vector2f(70, 35), sf::Vector2f(90, controlsY), "Pause", font, [this]() {
        if (currentDS) currentDS->pause();
    }));
    controlButtons.push_back(Button(sf::Vector2f(70, 35), sf::Vector2f(170, controlsY), "Step", font, [this]() {
        if (currentDS) currentDS->stepForward();
    }));
    controlButtons.push_back(Button(sf::Vector2f(230, 35), sf::Vector2f(10, controlsY + 45.f), isStepMode ? "Mode: Step by Step" : "Mode: Run at Once", font, [this]() {
        isStepMode = !isStepMode;
        controlButtons[3].setText(isStepMode ? "Mode: Step by Step" : "Mode: Run at Once");
        if (currentDS) currentDS->setStepMode(isStepMode);
    }));

    // Exit Button at bottom left
    controlButtons.push_back(Button(sf::Vector2f(230, 35), sf::Vector2f(10, controlsY + 90.f), "Exit", font, [this]() {
        showExitConfirm = true;
    }));

    // Confirmation Buttons
    float cx = windowWidth / 2.f;
    float cy = windowHeight / 2.f;
    exitConfirmButtons.push_back(Button(sf::Vector2f(100, 40), sf::Vector2f(cx - 120, cy + 20), "Yes", font, [this](){ window.close(); }));
    exitConfirmButtons.push_back(Button(sf::Vector2f(100, 40), sf::Vector2f(cx + 20, cy + 20), "No", font, [this](){ showExitConfirm = false; }));

    // Speed Slider
    speedSlider = std::make_unique<Slider>(sf::Vector2f(200, 4), sf::Vector2f(25, controlsY - 45.f), 0.1f, 2.0f, 0.6f, font, "Animation Speed");
    speedSlider->setOnValueChange([this](float val) {
        if (currentDS) currentDS->setPlayInterval(val);
    });
}

void App::buildCategoryButtons() {
    categoryButtons.clear();
    
    auto makeCatBtn = [this](int idx, const std::string& text, Category cat) {
        Button b(sf::Vector2f(110, 30), sf::Vector2f(10 + (idx % 2) * 120, 50 + (idx / 2) * 40), text, font, [this, cat]() {
            if (activeCategory != cat) {
                activeCategory = cat;
                uiNeedsUpdate = true;
                
                // Auto-select the first DS in the new category
                if (cat == Category::LINKED_LIST) {
                    activeDS = DSType::SINGLY_LINKED_LIST;
                    currentDS = std::make_unique<SinglyLinkedList>(windowWidth, windowHeight);
                } else if (cat == Category::HEAP) {
                    activeDS = DSType::MIN_MAX_HEAP;
                    currentDS = std::make_unique<MinMaxHeap>(windowWidth, windowHeight);
                } else if (cat == Category::TREE) {
                    activeDS = DSType::AVL_TREE;
                    currentDS = std::make_unique<AVLTree>(windowWidth, windowHeight);
                } else if (cat == Category::GRAPH) {
                    activeDS = DSType::KRUSKAL_MST;
                    currentDS = std::make_unique<GraphMST>(windowWidth, windowHeight, GraphMST::AlgoMode::KRUSKAL);
                }
                if (currentDS) currentDS->setStepMode(isStepMode);
            }
        });
        b.setActive(cat == activeCategory);
        categoryButtons.push_back(b);
    };

    makeCatBtn(0, "Linked List", Category::LINKED_LIST);
    makeCatBtn(1, "Heap",        Category::HEAP);
    makeCatBtn(2, "AVL Tree",    Category::TREE);
    makeCatBtn(3, "MST",         Category::GRAPH);
}

void App::buildDSButtons() {
    dsButtons.clear();
    
    float startY = 190.f;
    
    auto makeDSBtn = [this](float yPos, const std::string& text, DSType type, std::function<void()> onSelect) {
        Button b(sf::Vector2f(230, 40), sf::Vector2f(10, yPos), text, font, [this, type, onSelect]() {
            activeDS = type;
            uiNeedsUpdate = true;
            onSelect();
        });
        b.setActive(type == activeDS);
        dsButtons.push_back(b);
    };
    
    if (activeCategory == Category::LINKED_LIST) {
        makeDSBtn(startY, "Singly Linked List", DSType::SINGLY_LINKED_LIST, [this]() { currentDS = std::make_unique<SinglyLinkedList>(windowWidth, windowHeight); currentDS->setStepMode(isStepMode); });
    } else if (activeCategory == Category::HEAP) {
        makeDSBtn(startY, "Min/Max Heap", DSType::MIN_MAX_HEAP, [this]() { currentDS = std::make_unique<MinMaxHeap>(windowWidth, windowHeight); currentDS->setStepMode(isStepMode); });
    } else if (activeCategory == Category::TREE) {
        makeDSBtn(startY, "AVL Tree", DSType::AVL_TREE, [this]() { currentDS = std::make_unique<AVLTree>(windowWidth, windowHeight); currentDS->setStepMode(isStepMode); });
    } else if (activeCategory == Category::GRAPH) {
        auto updateMST = [this](GraphMST::AlgoMode mode, DSType dsType) {
            GraphMST* mst = dynamic_cast<GraphMST*>(currentDS.get());
            if (mst) {
                mst->setAlgoMode(mode);
            } else {
                currentDS = std::make_unique<GraphMST>(windowWidth, windowHeight, mode);
            }
            currentDS->setStepMode(isStepMode);
        };
        makeDSBtn(startY,       "Kruskal's Algorithm", DSType::KRUSKAL_MST, [this, updateMST]() { updateMST(GraphMST::AlgoMode::KRUSKAL, DSType::KRUSKAL_MST); });
        makeDSBtn(startY + 50.f, "Prim's Algorithm",    DSType::PRIM_MST,    [this, updateMST]() { updateMST(GraphMST::AlgoMode::PRIM,    DSType::PRIM_MST);    });
    }
}

void App::run() {
    while (window.isOpen()) {
        processEvents();
        update(deltaClock.restart().asSeconds());
        render();
    }
}

void App::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
        }

        if (event.type == sf::Event::Resized) {
            sf::FloatRect visibleArea(0, 0, (float)event.size.width, (float)event.size.height);
            window.setView(sf::View(visibleArea));
            windowWidth = (float)event.size.width;
            windowHeight = (float)event.size.height;
            sidebar.setSize(sf::Vector2f(250.f, windowHeight));
            initUI(); // Recalculate UI element positions based on new height/width
            if (currentDS) currentDS->onResize(windowWidth, windowHeight);
            window.setMouseCursorVisible(true);
        }

        if (event.type == sf::Event::GainedFocus) {
            window.setMouseCursorVisible(true);
        }

        if (showExitConfirm) {
            for (auto& btn : exitConfirmButtons) btn.handleEvent(event, window);
            continue; // block other inputs while confirming
        }

        for (auto& btn : categoryButtons) btn.handleEvent(event, window);
        for (auto& btn : dsButtons) btn.handleEvent(event, window);
        for (auto& btn : controlButtons) btn.handleEvent(event, window);
        if (speedSlider) speedSlider->handleEvent(event, window);

        if (currentDS) {
            currentDS->handleEvent(event, window);
        }
    }
}

void App::update(float dt) {
    if (dt > 0.05f) dt = 0.05f; // Prevent physics explosion (nodes disappearing) during lag spikes
    
    if (uiNeedsUpdate) {
        buildCategoryButtons();
        buildDSButtons();
        uiNeedsUpdate = false;
    }

    if (currentDS) {
        currentDS->update(dt);
    }
}

void App::render() {
    window.clear(sf::Color(30, 30, 30));

    if (currentDS) {
        currentDS->draw(window);
    }

    window.draw(sidebar);

    sf::Text title("Data Structures", font, 20);
    title.setPosition(10, 10);
    title.setFillColor(sf::Color::White);
    window.draw(title);
    
    sf::Text controlsTitle("Controls", font, 20);
    controlsTitle.setPosition(10, windowHeight - 190.f);
    controlsTitle.setFillColor(sf::Color::White);
    window.draw(controlsTitle);

    for (auto& btn : categoryButtons) btn.draw(window);
    for (auto& btn : dsButtons) btn.draw(window);
    for (auto& btn : controlButtons) btn.draw(window);
    if (speedSlider) speedSlider->draw(window);

    if (showExitConfirm) {
        sf::RectangleShape overlay(sf::Vector2f(windowWidth, windowHeight));
        overlay.setFillColor(sf::Color(0, 0, 0, 150));
        window.draw(overlay);

        sf::RectangleShape box(sf::Vector2f(400, 200));
        box.setFillColor(sf::Color(50, 50, 50));
        box.setOutlineThickness(2.f);
        box.setOutlineColor(sf::Color::White);
        box.setOrigin(200, 100);
        box.setPosition(windowWidth / 2.f, windowHeight / 2.f);
        window.draw(box);

        sf::Text qText("Are you sure to exit?", font, 20);
        qText.setFillColor(sf::Color::White);
        sf::FloatRect qb = qText.getLocalBounds();
        qText.setOrigin(qb.left + qb.width / 2.f, qb.top + qb.height / 2.f);
        qText.setPosition(windowWidth / 2.f, windowHeight / 2.f - 40.f);
        window.draw(qText);

        for (auto& btn : exitConfirmButtons) btn.draw(window);
    }

    window.display();
}
