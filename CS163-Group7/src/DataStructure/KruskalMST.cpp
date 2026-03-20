#include "DataStructure/KruskalMST.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// ---- Union-Find ----
void KruskalMST::UnionFind::init(int n) {
    parent.resize(n);
    rank_.resize(n, 0);
    for (int i = 0; i < n; ++i) parent[i] = i;
}

int KruskalMST::UnionFind::find(int x) {
    if (parent[x] != x) parent[x] = find(parent[x]);
    return parent[x];
}

bool KruskalMST::UnionFind::unite(int x, int y) {
    int px = find(x), py = find(y);
    if (px == py) return false;
    if (rank_[px] < rank_[py]) std::swap(px, py);
    parent[py] = px;
    if (rank_[px] == rank_[py]) rank_[px]++;
    return true;
}

// ---- KruskalMST ----
KruskalMST::KruskalMST()
    : kruskalStep(0), kruskalRunning(false), kruskalDone(false), totalMSTWeight(0)
{
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
        std::cerr << "Failed to load font\n";
    initUI();
}

KruskalMST::~KruskalMST() {}

void KruskalMST::initUI() {
    // --- Add Node ---
    textInputs.push_back(TextInput(sf::Vector2f(60, 30), sf::Vector2f(300, 560), font, "Node ID"));
    buttons.push_back(Button(sf::Vector2f(100, 30), sf::Vector2f(300, 600), "Add Node", font, [this]() {
        std::string s = textInputs[0].getText();
        if (!s.empty()) addNode(std::stoi(s));
    }));

    // --- Add Edge ---
    textInputs.push_back(TextInput(sf::Vector2f(40, 30), sf::Vector2f(410, 560), font, "From"));
    textInputs.push_back(TextInput(sf::Vector2f(40, 30), sf::Vector2f(460, 560), font, "To"));
    textInputs.push_back(TextInput(sf::Vector2f(40, 30), sf::Vector2f(510, 560), font, "Wt"));
    buttons.push_back(Button(sf::Vector2f(100, 30), sf::Vector2f(410, 600), "Add Edge", font, [this]() {
        std::string fs = textInputs[1].getText();
        std::string ts = textInputs[2].getText();
        std::string ws = textInputs[3].getText();
        if (!fs.empty() && !ts.empty() && !ws.empty())
            addEdge(std::stoi(fs), std::stoi(ts), std::stoi(ws));
    }));

    // --- Random Graph ---
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(520, 600), "Random Graph", font, [this]() {
        clear();
        int numNodes = 6;
        for (int i = 0; i < numNodes; ++i) addNode(i);
        // random edges
        for (int i = 0; i < numNodes; ++i)
            for (int j = i + 1; j < numNodes; ++j)
                if (rand() % 2) addEdge(i, j, rand() % 20 + 1);
        kruskalRunning = false;
        kruskalDone = false;
        kruskalStep = 0;
        for (auto& e : edges) { e.inMST = false; e.rejected = false; }
    }));

    // --- Run Kruskal ---
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(650, 600), "Run Kruskal", font, [this]() {
        runKruskal();
    }));

    // --- Step ---
    buttons.push_back(Button(sf::Vector2f(80, 30), sf::Vector2f(780, 600), "Step", font, [this]() {
        stepForward();
    }));

    // --- Clear ---
    buttons.push_back(Button(sf::Vector2f(80, 30), sf::Vector2f(870, 600), "Clear", font, [this]() {
        clear();
    }));
}

void KruskalMST::addNode(int id) {
    if (findNode(id) >= 0) return;
    GNode n;
    n.id = id;
    // Spread nodes in a circle in the canvas area
    float cx = 760.f, cy = 320.f, r = 200.f;
    float angle = (float)nodes.size() * 1.2f; // rough spread
    n.position = sf::Vector2f(cx + r * std::cos(angle), cy + r * std::sin(angle));
    nodes.push_back(n);
}

void KruskalMST::addEdge(int from, int to, int weight) {
    if (findNode(from) < 0 || findNode(to) < 0) return;
    edges.push_back({from, to, weight, false, false});
}

void KruskalMST::clear() {
    nodes.clear();
    edges.clear();
    sortedEdges.clear();
    kruskalRunning = false;
    kruskalDone = false;
    kruskalStep = 0;
    totalMSTWeight = 0;
}

void KruskalMST::runKruskal() {
    // Reset edge states
    for (auto& e : edges) { e.inMST = false; e.rejected = false; }
    totalMSTWeight = 0;

    // Sort by weight
    sortedEdges = edges;
    std::sort(sortedEdges.begin(), sortedEdges.end(), [](const Edge& a, const Edge& b) {
        return a.weight < b.weight;
    });

    // Init union-find over node indices
    uf.init((int)nodes.size());
    kruskalStep = 0;
    kruskalRunning = true;
    kruskalDone = false;
}

void KruskalMST::stepForward() {
    if (!kruskalRunning || kruskalDone) return;
    if (kruskalStep >= (int)sortedEdges.size()) {
        kruskalDone = true;
        kruskalRunning = false;
        return;
    }

    auto& se = sortedEdges[kruskalStep];
    int fi = findNode(se.from);
    int ti = findNode(se.to);

    if (fi >= 0 && ti >= 0 && uf.unite(fi, ti)) {
        se.inMST = true;
        totalMSTWeight += se.weight;
        // Reflect on original edges
        for (auto& e : edges) {
            if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
                e.inMST = true; break;
            }
        }
    } else {
        se.rejected = true;
        for (auto& e : edges) {
            if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
                e.rejected = true; break;
            }
        }
    }

    kruskalStep++;
    if (kruskalStep >= (int)sortedEdges.size()) {
        kruskalDone = true;
        kruskalRunning = false;
    }
}

int KruskalMST::findNode(int id) {
    for (int i = 0; i < (int)nodes.size(); ++i)
        if (nodes[i].id == id) return i;
    return -1;
}

void KruskalMST::applyForceLayout(float dt) {
    for (size_t i = 0; i < nodes.size(); ++i) {
        sf::Vector2f force(0, 0);
        for (size_t j = 0; j < nodes.size(); ++j) {
            if (i == j) continue;
            sf::Vector2f diff = nodes[i].position - nodes[j].position;
            float dist = std::sqrt(diff.x*diff.x + diff.y*diff.y);
            if (dist < 1.f) dist = 1.f;
            force += diff / (dist * dist) * 15000.f;
        }
        // Attract via edges
        for (auto& e : edges) {
            if (e.from == nodes[i].id) {
                int ti = findNode(e.to);
                if (ti >= 0) {
                    sf::Vector2f diff = nodes[ti].position - nodes[i].position;
                    force += diff * 0.01f;
                }
            }
            if (e.to == nodes[i].id) {
                int fi = findNode(e.from);
                if (fi >= 0) {
                    sf::Vector2f diff = nodes[fi].position - nodes[i].position;
                    force += diff * 0.01f;
                }
            }
        }
        // Center gravity toward canvas center
        sf::Vector2f center(760.f, 320.f);
        force += (center - nodes[i].position) * 0.05f;

        nodes[i].position += force * dt;

        // Clamp to canvas area [260..1260, 50..540]
        if (nodes[i].position.x < 280.f) nodes[i].position.x = 280.f;
        if (nodes[i].position.x > 1240.f) nodes[i].position.x = 1240.f;
        if (nodes[i].position.y < 60.f) nodes[i].position.y = 60.f;
        if (nodes[i].position.y > 520.f) nodes[i].position.y = 520.f;
    }
}

void KruskalMST::drawArrow(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to,
                            sf::Color color, int weight, bool inMST)
{
    sf::Vector2f dir = to - from;
    float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
    if (len < 1.f) return;
    dir /= len;

    float nodeR = 20.f;
    sf::Vector2f s = from + dir * nodeR;
    sf::Vector2f e = to - dir * nodeR;
    float lineLen = len - 2.f * nodeR;
    if (lineLen < 1.f) return;

    float thickness = inMST ? 4.f : 2.f;
    sf::RectangleShape line(sf::Vector2f(lineLen, thickness));
    line.setPosition(s);
    line.setFillColor(color);
    float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f;
    line.setRotation(angle);
    window.draw(line);

    // Weight label at midpoint
    sf::Vector2f mid = (from + to) / 2.f;
    sf::Text wText;
    wText.setFont(font);
    wText.setString(std::to_string(weight));
    wText.setCharacterSize(14);
    wText.setFillColor(sf::Color(255, 230, 100));
    sf::FloatRect wb = wText.getLocalBounds();
    wText.setOrigin(wb.left + wb.width/2.f, wb.top + wb.height/2.f);
    wText.setPosition(mid + sf::Vector2f(-dir.y, dir.x) * 14.f); // offset perpendicular
    window.draw(wText);
}

void KruskalMST::update(float dt) {
    applyForceLayout(dt);
}

void KruskalMST::draw(sf::RenderWindow& window) {
    float nodeR = 20.f;

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("MST - Kruskal's Algorithm");
    title.setCharacterSize(24);
    title.setFillColor(sf::Color::White);
    title.setPosition(300, 10);
    window.draw(title);

    // Status text
    if (kruskalDone) {
        sf::Text status;
        status.setFont(font);
        status.setString("MST Complete! Total Weight: " + std::to_string(totalMSTWeight));
        status.setCharacterSize(18);
        status.setFillColor(sf::Color(100, 255, 150));
        status.setPosition(300, 40);
        window.draw(status);
    } else if (kruskalRunning) {
        sf::Text status;
        status.setFont(font);
        status.setString("Step " + std::to_string(kruskalStep) + "/" + std::to_string((int)sortedEdges.size()) +
                         "  |  Press 'Step' to advance");
        status.setCharacterSize(16);
        status.setFillColor(sf::Color(180, 220, 255));
        status.setPosition(300, 40);
        window.draw(status);
    }

    // Draw edges
    for (auto& e : edges) {
        int fi = findNode(e.from);
        int ti = findNode(e.to);
        if (fi < 0 || ti < 0) continue;

        sf::Color edgeColor(150, 150, 150);
        bool inMST = false;
        if (e.inMST) { edgeColor = sf::Color(80, 220, 100); inMST = true; }
        else if (e.rejected) edgeColor = sf::Color(200, 80, 80);

        drawArrow(window, nodes[fi].position + sf::Vector2f(nodeR, nodeR),
                  nodes[ti].position + sf::Vector2f(nodeR, nodeR), edgeColor, e.weight, inMST);
    }

    // Draw nodes
    for (auto& n : nodes) {
        sf::CircleShape circle(nodeR);
        circle.setPosition(n.position);
        circle.setFillColor(sf::Color(40, 100, 180));
        circle.setOutlineThickness(3.f);
        circle.setOutlineColor(sf::Color(100, 180, 255));
        window.draw(circle);

        sf::Text idText;
        idText.setFont(font);
        idText.setString(std::to_string(n.id));
        idText.setCharacterSize(16);
        idText.setFillColor(sf::Color::White);
        sf::FloatRect tb = idText.getLocalBounds();
        idText.setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
        idText.setPosition(n.position + sf::Vector2f(nodeR, nodeR));
        window.draw(idText);
    }

    // Sorted edge list (step view)
    if (kruskalRunning || kruskalDone) {
        float listX = 300.f, listY = 70.f;
        sf::Text header;
        header.setFont(font);
        header.setString("Sorted Edges:");
        header.setCharacterSize(14);
        header.setFillColor(sf::Color(200,200,200));
        header.setPosition(listX, listY);
        window.draw(header);

        for (int i = 0; i < (int)sortedEdges.size() && i < 8; ++i) {
            auto& se = sortedEdges[i];
            std::string label = std::to_string(se.from) + "-" + std::to_string(se.to) +
                                " w=" + std::to_string(se.weight);
            sf::Color c(200, 200, 200);
            if (se.inMST) c = sf::Color(80, 220, 100);
            else if (se.rejected) c = sf::Color(200, 80, 80);
            if (i == kruskalStep && !kruskalDone) c = sf::Color(255, 230, 100);

            sf::Text et;
            et.setFont(font);
            et.setString(label);
            et.setCharacterSize(13);
            et.setFillColor(c);
            et.setPosition(listX, listY + 18.f + i * 18.f);
            window.draw(et);
        }
    }

    for (auto& btn : buttons) btn.draw(window);
    for (auto& inp : textInputs) inp.draw(window);
}

void KruskalMST::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    for (auto& btn : buttons) btn.handleEvent(event, window);
    for (auto& inp : textInputs) inp.handleEvent(event, window);
}

void KruskalMST::play() {}
void KruskalMST::pause() {}
void KruskalMST::stepBackward() {}
