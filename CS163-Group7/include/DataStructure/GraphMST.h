#pragma once
#include "DataStructure.h"
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "UI/Button.h"
#include "UI/TextInput.h"

// Graph MST (Kruskal's and Prim's) visualizer
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
    };

    struct Edge {
        int from, to, weight;
        bool inMST;    // true if selected by Kruskal
        bool rejected; // true if would form a cycle
    };

    // Union-Find helpers
    struct UnionFind {
        std::vector<int> parent, rank_;
        void init(int n);
        int find(int x);
        bool unite(int x, int y);
    };

    std::vector<GNode> nodes;
    std::vector<Edge> edges;

    std::vector<Edge> sortedEdges;
    UnionFind uf;
    int kruskalStep;       // current edge index being considered
    bool kruskalRunning;
    bool kruskalDone;
    
    // Prim step-by-step state
    std::vector<bool> primVisited;
    std::vector<Edge> primEdges; // Edges currently crossing the cut
    int primStep;
    bool primRunning;
    bool primDone;

    int totalMSTWeight;
    AlgoMode currentMode;

    bool isPlaying   = false;
    float playTimer  = 0.f;

    sf::Font font;
    std::vector<Button> buttons;
    std::vector<TextInput> textInputs;

    void initUI();
    int findNode(int id);
    void applyForceLayout(float dt);
    void drawArrow(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to,
                   sf::Color color, int weight, bool inMST);
};
