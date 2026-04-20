#pragma once
#include "DataStructure.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <queue>
#include <memory>
#include "UI/Button.h"
#include "UI/TextInput.h"
#include "UI/CodeHighlight.h"
#include "UI/InputDialog.h"

class GraphMST : public DataStructure {
public:
    enum class AlgoMode { KRUSKAL, PRIM };
    GraphMST(float windowWidth = 1280.f, float windowHeight = 720.f, AlgoMode mode = AlgoMode::KRUSKAL);
    ~GraphMST() override;

    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;

    void play() override;
    void pause() override;
    void stepForward() override;
    void stepBackward() override;
    void onResize(float w, float h) override;

    void addNode(int id);
    void addEdge(int from, int to, int weight);
    void clear();
    void runKruskal();
    void runPrim();
    void setAlgoMode(AlgoMode mode);

private:
    struct GNode {
        int id;
        sf::Vector2f position;
        bool locked = false;
    };

    struct Edge {
        int from, to, weight;
        bool inMST;    
        bool rejected; 
    };

    struct CompareEdge {
        bool operator()(const Edge& a, const Edge& b) const {
            return a.weight > b.weight;
        }
    };

    struct UnionFind {
        std::vector<int> parent, rank_;
        void init(int n);
        int find(int x);
        bool unite(int x, int y);
    };

    std::vector<GNode> nodes;
    std::vector<Edge> edges;

    std::vector<Edge> sortedEdges;
    std::vector<Edge> chosenEdges;
    UnionFind uf;
    int kruskalStep;       
    bool kruskalRunning;
    bool kruskalDone;
    
    std::vector<bool> primVisited;
    std::priority_queue<Edge, std::vector<Edge>, CompareEdge> primEdges;
    int primStep;
    bool primRunning;
    bool primDone;

    int totalMSTWeight;
    AlgoMode currentMode;

    bool isPlaying   = false;
    float playTimer  = 0.f;

    CodeHighlight codeViewer;
    std::vector<int> highlightedCodeLines;

    sf::Font font;
    std::vector<Button> buttons;
    std::vector<TextInput> textInputs;

    void initUI();
    int findNode(int id);
    void clampNodeToGraphArea(sf::Vector2f& pos) const;
    void applyForceLayout(float dt);
    void drawArrow(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to,
                   sf::Color color, int weight, bool inMST);

    std::unique_ptr<InputDialog> activeDialog;
    int draggedNodeIndex = -1;
    sf::Vector2f dragOffset;
};
