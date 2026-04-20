#include "DataStructure/MinMaxHeap.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

MinMaxHeap::MinMaxHeap(float windowWidth, float windowHeight) : isMinHeap(true) {
    winW = windowWidth;
    winH = windowHeight;
    if (!font.loadFromFile("assets/fonts/arial.ttf")) std::cerr << "Failed to load font\n";
    initUI();
}
MinMaxHeap::~MinMaxHeap() {}

void MinMaxHeap::initUI() {
    buttons.clear();
    textInputs.clear();
    
    float visualAreaCenter = 200.f + (winW - 200.f - codePaneWidth) / 2.f;
    float uiY1 = winH - 100.f; 
    float uiY2 = winH - 60.f;  
    
    // Total width roughly 850px
    float startX = visualAreaCenter - 425.f;
    if (startX < 210.f) startX = 210.f;

    textInputs.push_back(TextInput(sf::Vector2f(120,30), sf::Vector2f(startX, uiY1), font, "e.g. 10,20,30"));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(startX, uiY2), "Init Array", font, [this]() {
        std::string t = textInputs[0].getText(); std::vector<int> res; size_t pos = 0;
        while ((pos = t.find(',')) != std::string::npos) { if (pos>0) res.push_back(std::stoi(t.substr(0,pos))); t.erase(0,pos+1); }
        if (!t.empty()) res.push_back(std::stoi(t));
        if (!res.empty()) init(res);
    }));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(startX + 130, uiY2), "Init Random", font, [this]() {
        std::vector<int> r; for (int i=0;i<15;++i) r.push_back(rand()%100); init(r);
    }));

    textInputs.push_back(TextInput(sf::Vector2f(90,30), sf::Vector2f(startX + 260, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90,30), sf::Vector2f(startX + 260, uiY2), "Insert", font, [this]() {
        std::string v = textInputs[1].getText(); if (!v.empty()) beginInsertSteps(std::stoi(v));
    }));
    buttons.push_back(Button(sf::Vector2f(110,30), sf::Vector2f(startX + 360, uiY2), "Add Random", font, [this]() {
        beginInsertSteps(rand()%100);
    }));
    buttons.push_back(Button(sf::Vector2f(110,30), sf::Vector2f(startX + 480, uiY2), "Extract Root", font, [this]() {
        beginExtractSteps();
    }));
    textInputs.push_back(TextInput(sf::Vector2f(90,30), sf::Vector2f(startX + 600, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90,30), sf::Vector2f(startX + 600, uiY2), "Search", font, [this]() {
        std::string v = textInputs[2].getText(); if (!v.empty()) beginSearchSteps(std::stoi(v));
    }));
    buttons.push_back(Button(sf::Vector2f(130,30), sf::Vector2f(startX + 700, uiY2), "Toggle Min/Max", font, [this]() {
        isMinHeap = !isMinHeap; if (!rawData.empty()) buildHeap();
    }));
}

// ---- Step helpers ----
void MinMaxHeap::beginInsertSteps(int value) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;

    currentCode = {
        "void insert(int value) {",           // 0
        "  rawData.push_back(value);",        // 1
        "  heapifyUp(rawData.size() - 1);",   // 2
        "}",                                  // 3
        "void heapifyUp(int i) {",            // 4
        "  while (i > 0) {",                  // 5
        "    int p = (i-1)/2;",               // 6
        "    if (cmp(data[i], data[p])) {",    // 7
        "      swap(data[i], data[p]);",      // 8
        "      i = p;",                       // 9
        "    } else {",                       // 10
        "      break;",                       // 11
        "    }",                               // 12
        "  }",                                // 13
        "}"                                   // 14
    };

    if (!isStepByStep) {
        insert(value);
        VisualStep s; s.message = "Inserted " + std::to_string(value) + " (Run at Once)";
        s.highlightColor = sf::Color(0, 200, 80);
        s.codeLine = 0;
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    // Add node to end (not yet heapified)
    rawData.push_back(value);
    Node n; n.value = value;
    
    float rootX = 250.f + ((winW - 250.f - codePaneWidth) / 2.f);
    n.position = sf::Vector2f(rootX, winH + 100.f); 
    nodes.push_back(n);
    recalculateTargetPositions();
    int idx = (int)rawData.size() - 1;

    // Step 0: placed at end
    { VisualStep s; s.highlighted = {idx};
      s.message = "Placing " + std::to_string(value) + " at position [" + std::to_string(idx) + "]";
      s.highlightColor = sf::Color(80,200,255); 
      s.codeLine = 1;
      animSteps.push_back(s); }

    // Simulate heapify-up to record comparison steps
    std::vector<int> sim = rawData;
    int si = idx;
    while (si > 0) {
        int parent = (si - 1) / 2;
        bool doSwap = isMinHeap ? sim[si] < sim[parent] : sim[si] > sim[parent];
        
        // Highlight comparison
        VisualStep s; s.highlighted = {si, parent};
        s.codeLine = 7;
        s.message = "HeapifyUp: Checking if ["+std::to_string(si)+"]="+std::to_string(sim[si])+" should swap with parent ["+std::to_string(parent)+"]="+std::to_string(sim[parent]);
        animSteps.push_back(s);

        if (doSwap) {
            VisualStep s_swap; s_swap.highlighted = {si, parent};
            s_swap.message = "Swapping nodes";
            s_swap.highlightColor = sf::Color(0, 200, 80);
            s_swap.codeLine = 8;
            animSteps.push_back(s_swap);

            VisualStep s_i; s_i.highlighted = {parent};
            s_i.message = "Updating index i = p";
            s_i.codeLine = 9;
            animSteps.push_back(s_i);
            
            std::swap(sim[si], sim[parent]); si = parent;
        } else {
            VisualStep s_else; s_else.highlighted = {si, parent};
            s_else.message = "Condition false, heap property maintained. Entering else block...";
            s_else.codeLine = 10;
            animSteps.push_back(s_else);

            VisualStep s_break; s_break.highlighted = {si, parent};
            s_break.message = "Breaking loop";
            s_break.highlightColor = sf::Color(0,200,80); 
            s_break.codeLine = 11;
            animSteps.push_back(s_break);
            break;
        }
    }

    // Done notification step
    VisualStep done; done.message = "Inserted " + std::to_string(value) + "!";
    done.highlightColor = sf::Color(0, 200, 80);
    done.codeLine = 2;
    animSteps.push_back(done);

    // Commit: actual heapify
    int insertedIdx = idx;
    commitOp = [this, insertedIdx]() { heapifyUp(insertedIdx); recalculateTargetPositions(); };
    animStep = 0;
    isPlaying = true;
}

void MinMaxHeap::beginExtractSteps() {
    if (rawData.empty()) return;
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;

    currentCode = {
        "int extract() {",                    // 0
        "  int res = data[0];",               // 1
        "  data[0] = data.back();",           // 2
        "  data.pop_back();",                 // 3
        "  heapifyDown(0);",                  // 4
        "  return res;",                      // 5
        "}",                                  // 6
        "void heapifyDown(int i) {",          // 7
        "  while (true) {",                   // 8
        "    int smallest = findSmallest(i, i*2+1, i*2+2);", // 9
        "    if (smallest != i) {",           // 10
        "      swap(data[i], data[smallest]);", // 11
        "      i = smallest;",                // 12
        "    } else {",                       // 13
        "      break;",                       // 14
        "    }",                               // 15
        "  }",                                // 16
        "}"                                   // 17
    };

    if (!isStepByStep) {
        extract();
        VisualStep s; s.message = "Extracted root (Run at Once)";
        s.highlightColor = sf::Color(220, 60, 60);
        s.codeLine = 0;
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    int rootVal = rawData[0];
    int lastIdx = (int)rawData.size() - 1;

    // Step 0: highlight root
    { VisualStep s; s.highlighted = {0};
      s.message = "Extracting root: "+std::to_string(rootVal);
      s.highlightColor = sf::Color(220,60,60); 
      s.codeLine = 1;
      animSteps.push_back(s); }

    if (lastIdx > 0) {
        VisualStep s; s.highlighted = {0, lastIdx};
        s.message = "Moving last element ["+std::to_string(rawData[lastIdx])+"] to root position";
        s.codeLine = 2;
        animSteps.push_back(s);
    }

    // Simulate heapify-down
    std::vector<int> sim = rawData;
    sim[0] = sim.back(); sim.pop_back();
    int n = (int)sim.size(), si = 0;
    while (true) {
        int left=2*si+1, right=2*si+2, target=si;
        
        // Highlight line 9: findSmallest
        VisualStep s_find; s_find.highlighted = {si};
        if (left < n) s_find.highlighted.push_back(left);
        if (right < n) s_find.highlighted.push_back(right);
        s_find.message = "HeapifyDown: Finding smallest/largest among node and children";
        s_find.codeLine = 9;
        animSteps.push_back(s_find);

        if (left<n && (isMinHeap ? sim[left]<sim[target] : sim[left]>sim[target])) target=left;
        if (right<n && (isMinHeap ? sim[right]<sim[target] : sim[right]>sim[target])) target=right;
        
        // Highlight line 10: if (smallest != i)
        VisualStep s_if; s_if.highlighted = {si, target};
        s_if.message = "Checking if swap is needed (smallest/largest is " + std::to_string(target == si ? si : target) + ")";
        s_if.codeLine = 10;
        animSteps.push_back(s_if);

        if (target != si) {
            VisualStep s_swap; s_swap.highlighted = {si, target};
            s_swap.message = "Swapping nodes [" + std::to_string(sim[si]) + "] and [" + std::to_string(sim[target]) + "]";
            s_swap.highlightColor = sf::Color(0, 200, 80);
            s_swap.codeLine = 11;
            animSteps.push_back(s_swap);

            VisualStep s_i; s_i.highlighted = {target};
            s_i.message = "Updating index i = smallest";
            s_i.codeLine = 12;
            animSteps.push_back(s_i);
            
            std::swap(sim[si], sim[target]); si = target;
        } else {
            VisualStep s_else; s_else.highlighted = {si};
            s_else.message = "No swap needed. Entering else block...";
            s_else.codeLine = 13;
            animSteps.push_back(s_else);

            VisualStep s_break; s_break.highlighted = {si};
            s_break.message = "Breaking loop";
            s_break.highlightColor = sf::Color(0,200,80); 
            s_break.codeLine = 14;
            animSteps.push_back(s_break);
            break;
        }
    }
    
    // Done notification step
    VisualStep done; done.message = "Extracted root!";
    done.highlightColor = sf::Color(0, 150, 255);
    done.codeLine = 5;
    animSteps.push_back(done);

    commitOp = [this]() { extract(); };
    animStep = 0;
    isPlaying = true;
}

void MinMaxHeap::beginSearchSteps(int value) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;

    currentCode = {
        "bool search(int idx, int val) {",       // 0
        "  if (idx >= size) {",                  // 1
        "    return false;",                     // 2
        "  }",                                   // 3
        "  if (data[idx] == val) {",               // 4
        "    return true;",                      // 5
        "  }",                                   // 6
        "  if (impossible(data[idx], val)) {",     // 7
        "    return false;",                     // 8
        "  }",                                   // 9
        "  return search(idx*2+1, val) || search(idx*2+2, val);", // 10
        "}"                                      // 11
    };

    bool found = false;
    int foundIdx = -1;
    auto canContain = [&](int val) { return isMinHeap ? (val <= value) : (val >= value); };
    // ...

    if (!isStepByStep) {
        std::vector<int> stack;
        if (!rawData.empty() && canContain(rawData[0])) stack.push_back(0);
        while (!stack.empty()) {
            int idx = stack.back(); stack.pop_back();
            if (rawData[idx] == value) { found = true; foundIdx = idx; break; }
            int right = 2 * idx + 2;
            int left = 2 * idx + 1;
            if (right < (int)rawData.size() && canContain(rawData[right])) stack.push_back(right);
            if (left < (int)rawData.size() && canContain(rawData[left])) stack.push_back(left);
        }

        VisualStep s;
        if (found) {
            s.message = "Found " + std::to_string(value) + " at index " + std::to_string(foundIdx) + "! (Run at Once)";
            s.highlightColor = sf::Color(0, 200, 80);
            s.highlighted = {foundIdx};
        } else {
            s.message = std::to_string(value) + " not found in the heap. (Run at Once)";
            s.highlightColor = sf::Color(220, 60, 60);
        }
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    std::vector<int> stack;
    if (!rawData.empty()) {
        if (canContain(rawData[0])) {
            stack.push_back(0);
        } else {
            VisualStep s; s.highlighted = {0}; s.highlightColor = sf::Color(200, 150, 0);
            s.message = "Root is " + std::to_string(rawData[0]) + ", impossible to find " + std::to_string(value) + " below it. Pruned!";
            animSteps.push_back(s);
        }
    }

    while (!stack.empty()) {
        int idx = stack.back(); stack.pop_back();

        // Highlight line 4: if (data[idx] == val)
        VisualStep s_check; s_check.highlighted = {idx};
        s_check.message = "Checking if element [" + std::to_string(idx) + "] (" + std::to_string(rawData[idx]) + ") == " + std::to_string(value);
        s_check.codeLine = 4;
        animSteps.push_back(s_check);

        if (rawData[idx] == value) {
            VisualStep s_found; s_found.highlighted = {idx};
            s_found.message = "Found " + std::to_string(value) + "!";
            s_found.highlightColor = sf::Color(0, 200, 80);
            s_found.codeLine = 5;
            animSteps.push_back(s_found);
            found = true; foundIdx = idx; break;
        }

        // Highlight line 7: if (impossible(data[idx], val))
        VisualStep s_imp; s_imp.highlighted = {idx};
        s_imp.message = "Checking if " + std::to_string(value) + " is possible in this subtree...";
        s_imp.codeLine = 7;
        animSteps.push_back(s_imp);

        if (!canContain(rawData[idx])) {
            VisualStep s_prune; s_prune.highlighted = {idx};
            s_prune.message = "Value " + std::to_string(value) + " impossible under " + std::to_string(rawData[idx]) + ". Pruning.";
            s_prune.highlightColor = sf::Color(200, 150, 0);
            s_prune.codeLine = 8;
            animSteps.push_back(s_prune);
            continue;
        }

        // Highlight line 10: recursion
        VisualStep s_rec; s_rec.highlighted = {idx};
        s_rec.message = "Searching children of [" + std::to_string(idx) + "]";
        s_rec.codeLine = 10;
        animSteps.push_back(s_rec);

        int right = 2 * idx + 2;
        int left = 2 * idx + 1;
        if (right < (int)rawData.size()) stack.push_back(right);
        if (left < (int)rawData.size()) stack.push_back(left);
    }
    if (!found) {
        VisualStep s_fail;
        s_fail.message = std::to_string(value) + " not found in the heap.";
        s_fail.highlightColor = sf::Color(220, 60, 60);
        s_fail.codeLine = 2; // return false;
        animSteps.push_back(s_fail);
    }
    animStep = 0;
    isPlaying = true;
}

std::vector<std::string> MinMaxHeap::getCode() const { return currentCode; }
int MinMaxHeap::getCurrentLine() const { return (animStep >= 0 && animStep < (int)animSteps.size()) ? animSteps[animStep].codeLine : -1; }

// ---- play/pause/step ----
void MinMaxHeap::play()  { isPlaying = true; playTimer = 0.f; }
void MinMaxHeap::pause() { isPlaying = false; }
void MinMaxHeap::stepForward() {
    if (animStep < 0 || animSteps.empty()) return;
    if (animStep + 1 < (int)animSteps.size()) {
        animStep++;
        if (animStep == (int)animSteps.size() - 1) { // Transitioning to the final done step
            if (commitOp) { commitOp(); commitOp = nullptr; }
            isPlaying = false; // Stop auto-play and leave the message on screen
        }
    } else {
        isPlaying = false;
    }
}
void MinMaxHeap::stepBackward() { if (animStep > 0) animStep--; }

// ---- update ----
void MinMaxHeap::update(float dt) {
    for (auto& n : nodes) {
        n.position.x += (n.targetPosition.x - n.position.x) * 12.f * dt;
        n.position.y += (n.targetPosition.y - n.position.y) * 12.f * dt;
    }
    if (isPlaying && animStep >= 0) { playTimer += dt; if (playTimer >= playInterval) { playTimer = 0.f; stepForward(); } }
}

// ---- draw ----
void MinMaxHeap::draw(sf::RenderWindow& window) {
    float radius = 22.f;

    float titleX = 200.f + 50.f;
    if (animStep >= 0 && animStep < (int)animSteps.size()) {
        sf::Text msg; msg.setFont(font); 
        msg.setString(sf::String::fromUtf8(animSteps[animStep].message.begin(), animSteps[animStep].message.end()));
        msg.setCharacterSize(17); msg.setFillColor(sf::Color(200,230,255)); msg.setPosition(titleX, 40); window.draw(msg);
        sf::Text sc; sc.setFont(font); sc.setString("Step "+std::to_string(animStep+1)+"/"+std::to_string((int)animSteps.size()));
        sc.setCharacterSize(14); sc.setFillColor(sf::Color(160,160,160)); sc.setPosition(titleX, 60); window.draw(sc);
    }

    // Lines to children
    for (size_t i = 0; i < nodes.size(); ++i) {
        auto drawLine = [&](size_t child) {
            if (child >= nodes.size()) return;
            sf::Vector2f s = nodes[i].position + sf::Vector2f(radius,radius);
            sf::Vector2f e = nodes[child].position + sf::Vector2f(radius,radius);
            sf::Vector2f d = e - s; float len = std::sqrt(d.x*d.x+d.y*d.y);
            if (len > radius*2) {
                d/=len; s+=d*radius; e-=d*radius; len-=radius*2.f;
                sf::RectangleShape line(sf::Vector2f(len,2.f)); line.setPosition(s);
                line.setFillColor(sf::Color(150,150,150)); line.setRotation(std::atan2(d.y,d.x)*180.f/3.14159f); window.draw(line);
            }
        };
        drawLine(2*i+1); drawLine(2*i+2);
    }

    // Nodes
    for (size_t i = 0; i < nodes.size(); ++i) {
        sf::Color fillColor = isMinHeap ? sf::Color(0,150,100) : sf::Color(150,0,100);
        if (animStep >= 0 && animStep < (int)animSteps.size()) {
            const auto& step = animSteps[animStep];
            if (!step.highlighted.empty() && step.highlighted[0] == (int)i) fillColor = step.highlightColor;
            else if (step.highlighted.size() > 1 && step.highlighted[1] == (int)i) fillColor = step.highlightColor2;
        }
        sf::CircleShape circle(radius); circle.setPosition(nodes[i].position);
        circle.setFillColor(fillColor); circle.setOutlineThickness(2.f); circle.setOutlineColor(sf::Color::White); window.draw(circle);
        sf::Text txt; txt.setFont(font); txt.setString(std::to_string(nodes[i].value));
        txt.setCharacterSize(18); txt.setFillColor(sf::Color::White);
        sf::FloatRect tb = txt.getLocalBounds(); txt.setOrigin(tb.left+tb.width/2.f, tb.top+tb.height/2.f);
        txt.setPosition(nodes[i].position + sf::Vector2f(radius,radius)); window.draw(txt);
        sf::Text id; id.setFont(font); id.setString("["+std::to_string(i)+"]");
        id.setCharacterSize(12); id.setFillColor(sf::Color(200,200,200));
        id.setPosition(nodes[i].position.x+radius-10, nodes[i].position.y-20); window.draw(id);
    }

    sf::Text modeText; modeText.setFont(font); modeText.setString(isMinHeap ? "Min Heap" : "Max Heap");
    modeText.setCharacterSize(24); modeText.setFillColor(sf::Color::White); modeText.setPosition(titleX, 10); window.draw(modeText);

    for (auto& b : buttons) b.draw(window);
    for (auto& t : textInputs) t.draw(window);
}

void MinMaxHeap::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    for (auto& b : buttons) b.handleEvent(event, window);
    for (auto& t : textInputs) t.handleEvent(event, window);
}

void MinMaxHeap::init(const std::vector<int>& data) { rawData = data; buildHeap(); }

void MinMaxHeap::buildHeap() {
    nodes.clear();
    float rootX = 200.f + ((winW - 200.f - codePaneWidth) / 2.f);
    
    for (size_t i = 0; i < rawData.size(); ++i) {
        Node n; n.value = rawData[i]; n.position = sf::Vector2f(rootX, winH + 100.f); nodes.push_back(n);
    }
    for (int i = (int)rawData.size()/2 - 1; i >= 0; --i) heapifyDown(i);
    recalculateTargetPositions();
}

void MinMaxHeap::insert(int value) {
    rawData.push_back(value);
    float rootX = 200.f + ((winW - 200.f - codePaneWidth) / 2.f);
    Node n; n.value = value; n.position = sf::Vector2f(rootX, winH + 100.f); nodes.push_back(n);
    heapifyUp((int)rawData.size()-1); recalculateTargetPositions();
}

void MinMaxHeap::extract() {
    if (rawData.empty()) return;
    rawData[0] = rawData.back(); rawData.pop_back();
    nodes[0] = nodes.back(); nodes.pop_back();
    if (!rawData.empty()) heapifyDown(0);
    recalculateTargetPositions();
}

void MinMaxHeap::recalculateTargetPositions() {
    if (nodes.empty()) return;

    // Calculate dynamic base horizontal spread based on tree depth
    int maxDepth = (int)std::log2(nodes.size());
    // Give at least 200px spread, but grow aggressively if depth >= 4
    float initialSpread = std::max(200.f, std::pow(2.f, (float)maxDepth - 1.f) * 35.f);
    
    float rootX = 200.f + ((winW - 200.f - codePaneWidth) / 2.f);

    std::function<void(int, float, float, float)> calcPos = [&](int idx, float x, float y, float hSpread) {
        if (idx >= (int)nodes.size()) return;
        nodes[idx].targetPosition = {x, y};
        // Recursively lay out left child and right child, heavily halving the hSpread 
        calcPos(2 * idx + 1, x - hSpread, y + 80.f, hSpread / 2.f);
        calcPos(2 * idx + 2, x + hSpread, y + 80.f, hSpread / 2.f);
    };

    calcPos(0, rootX, 100.f, initialSpread);
}

static bool cmp(int a, int b, bool isMin) { return isMin ? a < b : a > b; }

void MinMaxHeap::heapifyUp(int index) {
    while (index > 0) {
        int parent = (index-1)/2;
        if (cmp(rawData[index], rawData[parent], isMinHeap)) {
            std::swap(rawData[index], rawData[parent]);
            std::swap(nodes[index], nodes[parent]);
            index = parent;
        } else break;
    }
}

void MinMaxHeap::heapifyDown(int index) {
    int size = (int)rawData.size();
    while (true) {
        int left=2*index+1, right=2*index+2, target=index;
        if (left<size && cmp(rawData[left],rawData[target],isMinHeap)) target=left;
        if (right<size && cmp(rawData[right],rawData[target],isMinHeap)) target=right;
        if (target != index) { std::swap(rawData[index],rawData[target]); std::swap(nodes[index],nodes[target]); index=target; }
        else break;
    }
}

void MinMaxHeap::onResize(float w, float h) {
    winW = w; winH = h;
    codePaneWidth = w / 6.0f;
    initUI();
    recalculateTargetPositions();
}
