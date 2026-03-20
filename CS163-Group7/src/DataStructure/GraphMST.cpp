#include "DataStructure/GraphMST.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

// ---- Union-Find ----
void GraphMST::UnionFind::init(int n) {
    parent.resize(n);
    rank_.resize(n, 0);
    for (int i = 0; i < n; ++i) parent[i] = i;
}

int GraphMST::UnionFind::find(int x) {
    if (parent[x] != x) parent[x] = find(parent[x]);
    return parent[x];
}

bool GraphMST::UnionFind::unite(int x, int y) {
    int px = find(x), py = find(y);
    if (px == py) return false;
    if (rank_[px] < rank_[py]) std::swap(px, py);
    parent[py] = px;
    if (rank_[px] == rank_[py]) rank_[px]++;
    return true;
}

// ---- GraphMST ----
GraphMST::GraphMST(float windowWidth, float windowHeight, AlgoMode mode)
    : kruskalStep(0), kruskalRunning(false), kruskalDone(false), 
      primStep(0), primRunning(false), primDone(false),
      totalMSTWeight(0), currentMode(mode)
{
    winW = windowWidth;
    winH = windowHeight;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
        std::cerr << "Failed to load font\n";
    initUI();
}

GraphMST::~GraphMST() {}

void GraphMST::initUI() {
    buttons.clear();
    textInputs.clear();

    float uiY1 = winH - 160.f;
    float uiY2 = winH - 120.f;

    // --- Add Node ---
    textInputs.push_back(TextInput(sf::Vector2f(60, 30), sf::Vector2f(300, uiY1), font, "Node ID"));
    buttons.push_back(Button(sf::Vector2f(100, 30), sf::Vector2f(300, uiY2), "Add Node", font, [this]() {
        std::string s = textInputs[0].getText();
        if (!s.empty()) addNode(std::stoi(s));
    }));

    // --- Add Edge ---
    textInputs.push_back(TextInput(sf::Vector2f(40, 30), sf::Vector2f(410, uiY1), font, "From"));
    textInputs.push_back(TextInput(sf::Vector2f(40, 30), sf::Vector2f(460, uiY1), font, "To"));
    textInputs.push_back(TextInput(sf::Vector2f(40, 30), sf::Vector2f(510, uiY1), font, "Wt"));
    buttons.push_back(Button(sf::Vector2f(100, 30), sf::Vector2f(410, uiY2), "Add Edge", font, [this]() {
        std::string fs = textInputs[1].getText();
        std::string ts = textInputs[2].getText();
        std::string ws = textInputs[3].getText();
        if (!fs.empty() && !ts.empty() && !ws.empty())
            addEdge(std::stoi(fs), std::stoi(ts), std::stoi(ws));
    }));

    // --- Random Graph ---
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(520, uiY2), "Random Graph", font, [this]() {
        clear();
        int numNodes = 6;
        for (int i = 0; i < numNodes; ++i) addNode(i);
        // random edges
        for (int i = 0; i < numNodes; ++i)
            for (int j = i + 1; j < numNodes; ++j)
                if (rand() % 2) addEdge(i, j, rand() % 20 + 1);
        kruskalRunning = false;
        kruskalDone = false;
        primRunning = false;
        primDone = false;
        kruskalStep = 0;
        primStep = 0;
        for (auto& e : edges) { e.inMST = false; e.rejected = false; }
    }));
    
    // --- Run MST ---
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(650, uiY2), (currentMode == GraphMST::AlgoMode::KRUSKAL ? "Run Kruskal" : "Run Prim"), font, [this]() {
        if (currentMode == GraphMST::AlgoMode::KRUSKAL) runKruskal();
        else runPrim();
    }));

    // --- Step ---
    buttons.push_back(Button(sf::Vector2f(80, 30), sf::Vector2f(780, uiY2), "Step", font, [this]() {
        stepForward();
    }));

    // --- Clear ---
    buttons.push_back(Button(sf::Vector2f(80, 30), sf::Vector2f(870, uiY2), "Clear", font, [this]() {
        clear();
    }));
}

void GraphMST::addNode(int id) {
    if (findNode(id) >= 0) return;
    GNode n;
    n.id = id;
    // Spread nodes in a circle in the canvas area
    float cx = 250.f + ((winW - 250.f) / 2.f); 
    float cy = winH / 2.f - 40.f;
    float r = std::min((winW - 250.f)/3.f, (winH - 200.f)/3.f);
    float angle = (float)nodes.size() * 1.2f; // rough spread
    n.position = sf::Vector2f(cx + r * std::cos(angle), cy + r * std::sin(angle));
    nodes.push_back(n);
}

void GraphMST::setAlgoMode(AlgoMode mode) {
    currentMode = mode;
    kruskalRunning = false;
    kruskalDone = false;
    primRunning = false;
    primDone = false;
    isPlaying = false;
    kruskalStep = 0;
    primStep = 0;
    totalMSTWeight = 0;
    sortedEdges.clear();
    primVisited.clear();
    primEdges.clear();
    for (auto& e : edges) { e.inMST = false; e.rejected = false; }
    
    // Update Run button text (buttons[3] is Run MST after I removed Toggle)
    if (buttons.size() > 3) {
        buttons[3].setText(currentMode == AlgoMode::KRUSKAL ? "Run Kruskal" : "Run Prim");
    }
}

void GraphMST::addEdge(int from, int to, int weight) {
    if (findNode(from) < 0 || findNode(to) < 0) return;
    edges.push_back({from, to, weight, false, false});
}

void GraphMST::clear() {
    nodes.clear();
    edges.clear();
    sortedEdges.clear();
    primEdges.clear();
    kruskalRunning = false;
    kruskalDone = false;
    primRunning = false;
    primDone = false;
    kruskalStep = 0;
    primStep = 0;
    totalMSTWeight = 0;
}

void GraphMST::runKruskal() {
    for (auto& e : edges) { e.inMST = false; e.rejected = false; }
    totalMSTWeight = 0;
    sortedEdges = edges;
    std::sort(sortedEdges.begin(), sortedEdges.end(), [](const Edge& a, const Edge& b) {
        return a.weight < b.weight;
    });
    uf.init((int)nodes.size());
    kruskalStep = 0;
    kruskalRunning = true;
    kruskalDone = false;
    primRunning = false;
    
    if (!isStepByStep) {
        while (!kruskalDone) stepForward();
        kruskalRunning = false;
    } else {
        isPlaying = true; // Auto play!
    }
}

void GraphMST::runPrim() {
    for (auto& e : edges) { e.inMST = false; e.rejected = false; }
    totalMSTWeight = 0;
    
    if (nodes.empty()) return;

    primVisited.assign(nodes.size(), false);
    primEdges.clear();
    
    // Start at node index 0
    primVisited[0] = true;
    for (const auto& e : edges) {
        if (findNode(e.from) == 0 || findNode(e.to) == 0) {
            primEdges.push_back(e);
        }
    }
    
    primStep = 0;
    primRunning = true;
    primDone = false;
    kruskalRunning = false;
    
    if (!isStepByStep) {
        while (!primDone) stepForward();
        primRunning = false;
    } else {
        isPlaying = true;
    }
}

void GraphMST::stepForward() {
    if (kruskalRunning && !kruskalDone) {
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
    } else if (primRunning && !primDone) {
        if (primEdges.empty()) {
            primDone = true;
            primRunning = false;
            return;
        }

        // Find minimum edge crossing the cut
        auto minIt = primEdges.begin();
        for (auto it = primEdges.begin(); it != primEdges.end(); ++it) {
            if (it->weight < minIt->weight) {
                minIt = it;
            }
        }

        Edge se = *minIt;
        primEdges.erase(minIt);

        int fi = findNode(se.from);
        int ti = findNode(se.to);

        // If one is visited and the other isn't, add to MST
        if (fi >= 0 && ti >= 0 && (primVisited[fi] ^ primVisited[ti])) {
            totalMSTWeight += se.weight;
            int newVisitedIdx = primVisited[fi] ? ti : fi;
            primVisited[newVisitedIdx] = true;

            for (auto& e : edges) {
                if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
                    e.inMST = true; break;
                }
            }

            // Add new crossing edges
            for (const auto& e : edges) {
                int ef = findNode(e.from);
                int et = findNode(e.to);
                if ((ef == newVisitedIdx || et == newVisitedIdx) && (primVisited[ef] ^ primVisited[et])) {
                    // Check if already in primEdges to avoid duplicates (optional, Prim's can handle duplicates)
                    primEdges.push_back(e);
                }
            }
        } else {
            // Both visited, reject
            for (auto& e : edges) {
                if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
                    e.rejected = true; break;
                }
            }
        }

        // Clean up internal queue: remove edges connecting two visited nodes
        primEdges.erase(std::remove_if(primEdges.begin(), primEdges.end(), [&](const Edge& e) {
            int fi = findNode(e.from);
            int ti = findNode(e.to);
            return (fi >= 0 && ti >= 0 && primVisited[fi] && primVisited[ti]);
        }), primEdges.end());

        primStep++;
        if (primEdges.empty()) {
            primDone = true;
            primRunning = false;
        }
    }
}

int GraphMST::findNode(int id) {
    for (int i = 0; i < (int)nodes.size(); ++i)
        if (nodes[i].id == id) return i;
    return -1;
}

void GraphMST::applyForceLayout(float dt) {
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
        float cx = 250.f + ((winW - 250.f) / 2.f);
        float cy = winH / 2.f - 40.f;
        sf::Vector2f center(cx, cy);
        force += (center - nodes[i].position) * 0.05f;

        nodes[i].position += force * dt;

        // Clamp to canvas area
        float minX = 280.f;
        float maxX = winW - 40.f;
        float minY = 60.f;
        float maxY = winH - 200.f;

        if (nodes[i].position.x < minX) nodes[i].position.x = minX;
        if (nodes[i].position.x > maxX) nodes[i].position.x = maxX;
        if (nodes[i].position.y < minY) nodes[i].position.y = minY;
        if (nodes[i].position.y > maxY) nodes[i].position.y = maxY;
    }
}

void GraphMST::drawArrow(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to,
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

void GraphMST::update(float dt) {
    applyForceLayout(dt);
    if (isPlaying && (kruskalRunning || primRunning)) {
        playTimer += dt;
        if (playTimer >= playInterval) { playTimer = 0.f; stepForward(); }
    }
    if (!kruskalRunning && !primRunning) isPlaying = false;
}

void GraphMST::draw(sf::RenderWindow& window) {
    float nodeR = 20.f;

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString(currentMode == GraphMST::AlgoMode::KRUSKAL ? "MST - Kruskal's Algorithm" : "MST - Prim's Algorithm");
    title.setCharacterSize(24);
    title.setFillColor(sf::Color::White);
    title.setPosition(300, 10);
    window.draw(title);

    // Status text
    if (kruskalDone || primDone) {
        sf::Text status;
        status.setFont(font);
        status.setString("MST Complete! Total Weight: " + std::to_string(totalMSTWeight));
        status.setCharacterSize(18);
        status.setFillColor(sf::Color(100, 255, 150));
        status.setPosition(300, 40);
        window.draw(status);
    } else if (kruskalRunning || primRunning) {
        sf::Text status;
        status.setFont(font);
        int currentStep = kruskalRunning ? kruskalStep : primStep;
        status.setString("Step " + std::to_string(currentStep) + "  |  Press 'Step' to advance");
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
    for (size_t i = 0; i < nodes.size(); ++i) {
        auto& n = nodes[i];
        sf::CircleShape circle(nodeR);
        circle.setPosition(n.position);
        
        // Highlight visited nodes in Prim
        if (primRunning || primDone) {
            if (i < primVisited.size() && primVisited[i]) circle.setFillColor(sf::Color(100, 180, 100));
            else circle.setFillColor(sf::Color(40, 100, 180));
        } else {
            circle.setFillColor(sf::Color(40, 100, 180));
        }
        
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

    // Graph Edge Info list (step view)
    if (kruskalRunning || kruskalDone) {
        float listX = 300.f, listY = 70.f;
        sf::Text header;
        header.setFont(font);
        header.setString("Sorted Edges (Kruskal):");
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
    } else if (primRunning || primDone) {
        float listX = 300.f, listY = 70.f;
        sf::Text header;
        header.setFont(font);
        header.setString("Cut Edges (Prim):");
        header.setCharacterSize(14);
        header.setFillColor(sf::Color(200,200,200));
        header.setPosition(listX, listY);
        window.draw(header);

        auto pEdgesSorted = primEdges;
        std::sort(pEdgesSorted.begin(), pEdgesSorted.end(), [](const Edge& a, const Edge& b){
            return a.weight < b.weight;
        });
        for (int i = 0; i < (int)pEdgesSorted.size() && i < 8; ++i) {
            auto& se = pEdgesSorted[i];
            std::string label = std::to_string(se.from) + "-" + std::to_string(se.to) +
                                " w=" + std::to_string(se.weight);
            sf::Color c = (i == 0 && !primDone) ? sf::Color(255, 230, 100) : sf::Color(200, 200, 200); // Highlight cheapest

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

void GraphMST::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    for (auto& btn : buttons) btn.handleEvent(event, window);
    for (auto& inp : textInputs) inp.handleEvent(event, window);
}

void GraphMST::play()  { isPlaying = true; playTimer = 0.f; if (!kruskalRunning && !primRunning) { if (currentMode == GraphMST::AlgoMode::KRUSKAL) runKruskal(); else runPrim(); } }
void GraphMST::pause() { isPlaying = false; }
void GraphMST::stepBackward() {}

void GraphMST::onResize(float w, float h) {
    winW = w; winH = h;
    initUI();
}
