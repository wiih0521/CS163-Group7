#include "DataStructure/SinglyLinkedList.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>

SinglyLinkedList::SinglyLinkedList(float windowWidth, float windowHeight) {
    winW = windowWidth;
    winH = windowHeight;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
        std::cerr << "Failed to load font\n";
    initUI();
}
SinglyLinkedList::~SinglyLinkedList() {}

void SinglyLinkedList::initUI() {
    buttons.clear();
    textInputs.clear();
    
    float visualAreaCenter = 200.f + (winW - 200.f - codePaneWidth) / 2.f;
    float uiY1 = winH - 100.f;
    float uiY2 = winH - 60.f;
    
    // Total width roughly 780px
    float startX = visualAreaCenter - 390.f;
    if (startX < 210.f) startX = 210.f;

    textInputs.push_back(TextInput(sf::Vector2f(120, 30), sf::Vector2f(startX, uiY1), font, "e.g. 10,20,30"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(startX, uiY2), "Init Array", font, [this]() {
        std::string t = textInputs[0].getText();
        std::vector<int> res; size_t pos = 0;
        while ((pos = t.find(',')) != std::string::npos) { if (pos > 0) res.push_back(std::stoi(t.substr(0, pos))); t.erase(0, pos + 1); }
        if (!t.empty()) res.push_back(std::stoi(t));
        if (!res.empty()) init(res);
    }));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(startX + 130, uiY2), "Init Random", font, [this]() {
        std::vector<int> r; for (int i = 0; i < 5; ++i) r.push_back(rand() % 100); init(r);
    }));

    textInputs.push_back(TextInput(sf::Vector2f(60, 30), sf::Vector2f(startX + 260, uiY1), font, "Value"));
    textInputs.push_back(TextInput(sf::Vector2f(50, 30), sf::Vector2f(startX + 330, uiY1), font, "Idx"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(startX + 260, uiY2), "Add Node", font, [this]() {
        std::string v = textInputs[1].getText(), i = textInputs[2].getText();
        if (!v.empty()) beginInsertSteps(std::stoi(v), i.empty() ? (int)nodes.size() : std::stoi(i));
    }));
    buttons.push_back(Button(sf::Vector2f(110, 30), sf::Vector2f(startX + 390, uiY2), "Add Random", font, [this]() {
        beginInsertSteps(rand() % 100, (int)nodes.size());
    }));

    textInputs.push_back(TextInput(sf::Vector2f(120, 30), sf::Vector2f(startX + 510, uiY1), font, "Index to Del"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(startX + 510, uiY2), "Delete Node", font, [this]() {
        std::string i = textInputs[3].getText();
        if (!i.empty()) beginDeleteSteps(std::stoi(i));
    }));

    textInputs.push_back(TextInput(sf::Vector2f(80, 30), sf::Vector2f(startX + 640, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(80, 30), sf::Vector2f(startX + 640, uiY2), "Search", font, [this]() {
        std::string v = textInputs[4].getText();
        if (!v.empty()) beginSearchSteps(std::stoi(v));
    }));
}

// ---- Step-by-step helpers ----
void SinglyLinkedList::beginInsertSteps(int value, int idx) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;
    if (idx < 0) idx = 0;
    if (idx > (int)nodes.size()) idx = (int)nodes.size();
    
    currentCode = {
        "void insert(int value, int idx) {",         // 0
        "  Node* newNode = new Node(value);",        // 1
        "  if (idx == 0) {",                         // 2
        "    newNode->next = head;",                 // 3
        "    head = newNode;",                       // 4
        "    return;",                               // 5
        "  }",                                       // 6
        "  Node* curr = head;",                      // 7
        "  for (int i = 0; i < idx - 1; ++i) {",     // 8
        "    curr = curr->next;",                    // 9
        "  }",                                       // 10
        "  newNode->next = curr->next;",             // 11
        "  curr->next = newNode;",                   // 12
        "}"                                          // 13
    };

    if (!isStepByStep) {
        insert(value, idx);
        VisualStep s; s.message = "Inserted " + std::to_string(value) + " at index " + std::to_string(idx) + " (Run at Once)";
        s.highlightColor = sf::Color(0, 200, 80);
        if (idx < (int)nodes.size()) s.highlighted = {idx};
        s.codeLine = -1;
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    // Step 1: Create newNode
    VisualStep s1; s1.message = "Created new node with value " + std::to_string(value);
    s1.codeLine = 1;
    animSteps.push_back(s1);

    // ALWAYS highlight the if check
    VisualStep s_if; s_if.message = "Checking if (idx == 0)...";
    s_if.codeLine = 2;
    animSteps.push_back(s_if);

    if (idx == 0) {
        VisualStep s3; s3.message = "newNode->next = head";
        s3.codeLine = 3;
        animSteps.push_back(s3);
        
        VisualStep s4; s4.message = "head = newNode";
        s4.codeLine = 4;
        animSteps.push_back(s4);
        
        VisualStep s_ret; s_ret.message = "Returning...";
        s_ret.codeLine = 5;
        animSteps.push_back(s_ret);
    } else {
        VisualStep s2; s2.message = "Index != 0, finding position to insert...";
        s2.codeLine = 7;
        animSteps.push_back(s2);

        for (int i = 0; i < idx - 1 && i < (int)nodes.size(); ++i) {
            VisualStep s_for; s_for.message = "for: i = " + std::to_string(i);
            s_for.codeLine = 8;
            animSteps.push_back(s_for);

            VisualStep s; s.highlighted = {i};
            s.message = "Traversing... current node is " + std::to_string(nodes[i].value);
            s.codeLine = 9;
            animSteps.push_back(s);
        }
        
        VisualStep s4; s4.message = "Found insertion point. Updating pointers...";
        s4.codeLine = 11;
        animSteps.push_back(s4);

        VisualStep s5; s5.message = "curr->next = newNode";
        s5.codeLine = 12;
        animSteps.push_back(s5);
    }

    VisualStep fin;
    fin.message = "Inserted " + std::to_string(value) + " at index " + std::to_string(idx) + "!";
    fin.highlightColor = sf::Color(0, 200, 80);
    fin.codeLine = 13;
    animSteps.push_back(fin);
    
    commitOp = [this, value, idx]() { insert(value, idx); };
    animStep = 0;
    isPlaying = true; 
}

void SinglyLinkedList::beginDeleteSteps(int idx) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;
    if (idx < 0 || idx >= (int)nodes.size()) return;
    int value = nodes[idx].value;

    currentCode = {
        "void remove(int idx) {",                    // 0
        "  if (idx == 0) {",                         // 1
        "    Node* temp = head;",                    // 2
        "    head = head->next;",                    // 3
        "    delete temp;",                          // 4
        "    return;",                               // 5
        "  }",                                       // 6
        "  Node* curr = head;",                      // 7
        "  for (int i = 0; i < idx - 1; ++i) {",     // 8
        "    curr = curr->next;",                    // 9
        "  }",                                       // 10
        "  Node* temp = curr->next;",                // 11
        "  curr->next = temp->next;",                // 12
        "  delete temp;",                            // 13
        "}"                                          // 14
    };

    if (!isStepByStep) {
        remove(idx);
        VisualStep s; s.message = "Deleted node at index " + std::to_string(idx) + " with value " + std::to_string(value) + " (Run at Once)";
        s.highlightColor = sf::Color(220, 60, 60);
        s.codeLine = -1;
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    // ALWAYS highlight the if check
    VisualStep s_if; s_if.message = "Checking if (idx == 0)...";
    s_if.codeLine = 1;
    animSteps.push_back(s_if);

    if (idx == 0) {
        VisualStep s1; s1.message = "Removing head node";
        s1.codeLine = 2;
        animSteps.push_back(s1);
        
        VisualStep s2; s2.message = "Updating head pointer";
        s2.codeLine = 3;
        animSteps.push_back(s2);

        VisualStep s3; s3.message = "Deleting old head...";
        s3.codeLine = 4;
        animSteps.push_back(s3);

        VisualStep s_ret; s_ret.message = "Returning...";
        s_ret.codeLine = 5;
        animSteps.push_back(s_ret);
    } else {
        VisualStep s1; s1.message = "Finding node at index " + std::to_string(idx);
        s1.codeLine = 7;
        animSteps.push_back(s1);

        for (int i = 0; i < idx - 1; ++i) {
            VisualStep s_for; s_for.message = "for: i = " + std::to_string(i);
            s_for.codeLine = 8;
            animSteps.push_back(s_for);

            VisualStep s; s.highlighted = {i};
            s.message = "Searching... at node[" + std::to_string(i) + "] = " + std::to_string(nodes[i].value);
            s.codeLine = 9;
            animSteps.push_back(s);
        }

        VisualStep s3; s3.highlighted = {idx};
        s3.message = "Found node. Updating pointers and deleting...";
        s3.highlightColor = sf::Color(220, 60, 60);
        s3.codeLine = 11;
        animSteps.push_back(s3);

        VisualStep s4; s4.message = "curr->next = temp->next";
        s4.codeLine = 12;
        animSteps.push_back(s4);

        VisualStep s5; s5.message = "delete temp";
        s5.codeLine = 13;
        animSteps.push_back(s5);
    }

    VisualStep done; 
    done.message = "Deleted node at index " + std::to_string(idx) + "!";
    done.highlightColor = sf::Color(220, 60, 60);
    done.codeLine = 14;
    animSteps.push_back(done);

    commitOp = [this, idx]() { remove(idx); };
    animStep = 0;
    isPlaying = true;
}

void SinglyLinkedList::beginSearchSteps(int value) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;
    bool found = false;
    int foundIdx = -1;

    currentCode = {
        "bool search(int value) {",                  // 0
        "  Node* curr = head;",                      // 1
        "  while (curr != nullptr) {",                // 2
        "    if (curr->value == value) {",            // 3
        "      return true;",                        // 4
        "    }",                                     // 5
        "    curr = curr->next;",                    // 6
        "  }",                                       // 7
        "  return false;",                           // 8
        "}"                                          // 9
    };

    if (!isStepByStep) {
        for (int i = 0; i < (int)nodes.size(); ++i) {
            if (nodes[i].value == value) { found = true; foundIdx = i; break; }
        }
        VisualStep s;
        if (found) {
            s.message = "Found " + std::to_string(value) + " at index " + std::to_string(foundIdx) + "! (Run at Once)";
            s.highlightColor = sf::Color(0, 200, 80);
            s.highlighted = {foundIdx};
        } else {
            s.message = std::to_string(value) + " not found in the list. (Run at Once)";
            s.highlightColor = sf::Color(220, 60, 60);
        }
        s.codeLine = -1;
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    VisualStep s1; s1.message = "Starting search for " + std::to_string(value);
    s1.codeLine = 1;
    animSteps.push_back(s1);

    for (int i = 0; i < (int)nodes.size(); ++i) {
        VisualStep s_while; s_while.message = "while (curr != nullptr)";
        s_while.codeLine = 2;
        animSteps.push_back(s_while);

        VisualStep s_check; s_check.highlighted = {i};
        s_check.message = "Checking if (curr->value == value)...";
        s_check.codeLine = 4;
        animSteps.push_back(s_check);

        if (nodes[i].value == value) {
            VisualStep s_found; s_found.highlighted = {i};
            s_found.message = "Found " + std::to_string(value) + "!";
            s_found.highlightColor = sf::Color(0, 200, 80);
            s_found.codeLine = 5;
            animSteps.push_back(s_found);
            found = true; foundIdx = i;
            break;
        } else {
            VisualStep s_next; s_next.highlighted = {i};
            s_next.message = "Condition false, moving to next...";
            s_next.codeLine = 6;
            animSteps.push_back(s_next);
        }
    }
    
    if (!found) {
        VisualStep s_while_f; s_while_f.message = "while loop ended (curr is nullptr)";
        s_while_f.codeLine = 2;
        animSteps.push_back(s_while_f);

        VisualStep s_fail;
        s_fail.message = std::to_string(value) + " not found.";
        s_fail.highlightColor = sf::Color(220, 60, 60);
        s_fail.codeLine = 8; // return false;
        animSteps.push_back(s_fail);
    }
    animStep = 0;
    isPlaying = true;
}

// ---- Core play/pause/step ----
void SinglyLinkedList::play()  { isPlaying = true; playTimer = 0.f; }
void SinglyLinkedList::pause() { isPlaying = false; }

void SinglyLinkedList::stepForward() {
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
void SinglyLinkedList::stepBackward() { if (animStep > 0) animStep--; }

// ---- update ----
void SinglyLinkedList::update(float dt) {
    for (auto& n : nodes) {
        if (std::isnan(n.position.x) || std::isnan(n.position.y)) n.position = sf::Vector2f(350.f, -50.f);
        if (std::isnan(n.targetPosition.x) || std::isnan(n.targetPosition.y)) n.targetPosition = sf::Vector2f(350.f, 300.f);
        n.position.x += (n.targetPosition.x - n.position.x) * 12.f * dt;
        n.position.y += (n.targetPosition.y - n.position.y) * 12.f * dt;
    }
    if (isPlaying && animStep >= 0) {
        playTimer += dt;
        if (playTimer >= playInterval) { playTimer = 0.f; stepForward(); }
    }
}

// ---- draw ----
void SinglyLinkedList::draw(sf::RenderWindow& window) {
    float titleX = 200.f + 50.f;
    sf::Text title; title.setFont(font); title.setString("Singly Linked List");
    title.setCharacterSize(24); title.setFillColor(sf::Color::White); title.setPosition(titleX, 10);
    window.draw(title);

    // Step message
    if (animStep >= 0 && animStep < (int)animSteps.size()) {
        sf::Text msg; msg.setFont(font); 
        msg.setString(sf::String::fromUtf8(animSteps[animStep].message.begin(), animSteps[animStep].message.end()));
        msg.setCharacterSize(17); msg.setFillColor(sf::Color(200, 230, 255)); msg.setPosition(titleX, 40);
        window.draw(msg);
        // Step counter
        sf::Text sc; sc.setFont(font);
        sc.setString("Step " + std::to_string(animStep + 1) + "/" + std::to_string((int)animSteps.size()));
        sc.setCharacterSize(14); sc.setFillColor(sf::Color(160, 160, 160)); sc.setPosition(titleX, 60);
        window.draw(sc);
    }

    float radius = 25.f;
    for (size_t i = 0; i < nodes.size(); ++i) {
        // Arrow to next
        if (i < nodes.size() - 1) {
            sf::Vector2f s = nodes[i].position + sf::Vector2f(radius, radius);
            sf::Vector2f e = nodes[i+1].position + sf::Vector2f(radius, radius);
            sf::Vector2f d = e - s; float len = std::sqrt(d.x*d.x + d.y*d.y);
            if (len > radius * 2) {
                d /= len; s += d*radius; e -= d*radius; len -= radius*2.f;
                sf::RectangleShape line(sf::Vector2f(len, 2.f));
                line.setPosition(s); line.setFillColor(sf::Color::White);
                float angle = std::atan2(d.y, d.x)*180.f/3.14159f; line.setRotation(angle);
                window.draw(line);
                sf::ConvexShape arr(3); arr.setPoint(0,{0,0}); arr.setPoint(1,{-10,5}); arr.setPoint(2,{-10,-5});
                arr.setFillColor(sf::Color::White); arr.setPosition(e); arr.setRotation(angle);
                window.draw(arr);
            }
        }
        // Determine node color from step state
        sf::Color fillColor(0, 120, 215);
        if (animStep >= 0 && animStep < (int)animSteps.size()) {
            for (int hi : animSteps[animStep].highlighted)
                if (hi == (int)i) { fillColor = animSteps[animStep].highlightColor; break; }
        }
        sf::CircleShape circle(radius); circle.setPosition(nodes[i].position);
        circle.setFillColor(fillColor); circle.setOutlineThickness(2.f); circle.setOutlineColor(sf::Color::White);
        window.draw(circle);

        sf::Text txt; txt.setFont(font); txt.setString(std::to_string(nodes[i].value));
        txt.setCharacterSize(20); txt.setFillColor(sf::Color::White);
        sf::FloatRect tb = txt.getLocalBounds(); txt.setOrigin(tb.left+tb.width/2.f, tb.top+tb.height/2.f);
        txt.setPosition(nodes[i].position + sf::Vector2f(radius, radius)); window.draw(txt);

        sf::Text idx; idx.setFont(font); idx.setString(std::to_string(i));
        idx.setCharacterSize(14); idx.setFillColor(sf::Color(200,200,200));
        idx.setPosition(nodes[i].position.x+radius-5, nodes[i].position.y+radius*2.f+5);
        window.draw(idx);
    }
    for (auto& b : buttons) b.draw(window);
    for (auto& t : textInputs) t.draw(window);
}

void SinglyLinkedList::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    for (auto& b : buttons) b.handleEvent(event, window);
    for (auto& t : textInputs) t.handleEvent(event, window);
}

void SinglyLinkedList::init(const std::vector<int>& data) {
    nodes.clear();
    
    float totalWidth = data.size() * 100.f;
    float startX = 200.f + ((winW - 200.f - codePaneWidth - totalWidth) / 2.f); // Base offset from sidebar and code pane
    
    for (size_t i = 0; i < data.size(); ++i) {
        Node n; n.value = data[i];
        n.targetPosition = sf::Vector2f(startX + i*100.f, winH / 2.f - 50.f);
        n.position = sf::Vector2f(startX + i*100.f, -50.f);
        nodes.push_back(n);
    }
}
void SinglyLinkedList::insert(int value, int index) {
    if (index < 0) index = 0;
    if (index > (int)nodes.size()) index = (int)nodes.size();
    Node n; n.value = value;
    
    n.position = sf::Vector2f(200.f + (winW - 200.f - codePaneWidth) / 2.f, winH + 100.f);
    nodes.insert(nodes.begin() + index, n);
    
    float totalWidth = nodes.size() * 100.f;
    float startX = 250.f + ((winW - 250.f - codePaneWidth - totalWidth) / 2.f);
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
    }
}
void SinglyLinkedList::remove(int index) {
    if (index < 0 || index >= (int)nodes.size()) return;
    nodes.erase(nodes.begin() + index);
    
    float totalWidth = nodes.size() * 100.f;
    float startX = 200.f + ((winW - 200.f - codePaneWidth - totalWidth) / 2.f);
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
    }
}
void SinglyLinkedList::updateNode(int index, int newValue) {}

void SinglyLinkedList::onResize(float w, float h) {
    winW = w; winH = h;
    codePaneWidth = w / 6.0f;
    initUI();
    // Re-layout existing nodes
    if (!nodes.empty()) {
        float totalWidth = nodes.size() * 100.f;
        float startX = 200.f + ((winW - 200.f - codePaneWidth - totalWidth) / 2.f);
        for (size_t i = 0; i < nodes.size(); ++i) {
            nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
        }
    }
}

std::vector<std::string> SinglyLinkedList::getCode() const {
    return currentCode;
}

int SinglyLinkedList::getCurrentLine() const {
    if (animStep >= 0 && animStep < (int)animSteps.size()) {
        return animSteps[animStep].codeLine;
    }
    return -1;
}
