#include "DataStructure/LinkedList.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <string>

LinkedList::LinkedList(float windowWidth, float windowHeight) {
    winW = windowWidth;
    winH = windowHeight;
    if (!font.loadFromFile("assets/fonts/arial.ttf"))
        std::cerr << "Failed to load font\n";
    initUI();
}

LinkedList::~LinkedList() {}

void LinkedList::initUI() {
    buttons.clear();
    textInputs.clear();

    float uiY1 = winH - 160.f;
    float uiY2 = winH - 120.f;

    textInputs.push_back(TextInput(sf::Vector2f(120, 30), sf::Vector2f(300, uiY1), font, "e.g. 10,20,30"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(300, uiY2), "Init Array", font, [this]() {
        std::string t = textInputs[0].getText();
        std::vector<int> res; size_t pos = 0;
        while ((pos = t.find(',')) != std::string::npos) { if (pos > 0) res.push_back(std::stoi(t.substr(0, pos))); t.erase(0, pos + 1); }
        if (!t.empty()) res.push_back(std::stoi(t));
        if (!res.empty()) init(res);
    }));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(430, uiY2), "Init Random", font, [this]() {
        std::vector<int> r; for (int i = 0; i < 5; ++i) r.push_back(rand() % 100); init(r);
    }));
    textInputs.push_back(TextInput(sf::Vector2f(60, 30), sf::Vector2f(560, uiY1), font, "Value"));
    textInputs.push_back(TextInput(sf::Vector2f(50, 30), sf::Vector2f(630, uiY1), font, "Idx"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(560, uiY2), "Add Node", font, [this]() {
        std::string v = textInputs[1].getText(), i = textInputs[2].getText();
        if (!v.empty()) beginInsertSteps(std::stoi(v), i.empty() ? (int)nodes.size() : std::stoi(i));
    }));
    buttons.push_back(Button(sf::Vector2f(110, 30), sf::Vector2f(690, uiY2), "Add Random", font, [this]() {
        beginInsertSteps(rand() % 100, (int)nodes.size());
    }));
    textInputs.push_back(TextInput(sf::Vector2f(120, 30), sf::Vector2f(810, uiY1), font, "Index to Del"));
    buttons.push_back(Button(sf::Vector2f(120, 30), sf::Vector2f(810, uiY2), "Delete Node", font, [this]() {
        std::string i = textInputs[3].getText();
        if (!i.empty()) beginDeleteSteps(std::stoi(i));
    }));
    textInputs.push_back(TextInput(sf::Vector2f(90, 30), sf::Vector2f(940, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90, 30), sf::Vector2f(940, uiY2), "Search", font, [this]() {
        std::string v = textInputs[4].getText();
        if (!v.empty()) beginSearchSteps(std::stoi(v));
    }));
}

void LinkedList::init(const std::vector<int>& data) {
    nodes.clear();
    float totalWidth = data.size() * 100.f;
    float startX = 250.f + ((winW - 250.f - totalWidth) / 2.f);
    for (size_t i = 0; i < data.size(); ++i) {
        Node n; n.value = data[i];
        n.targetPosition = sf::Vector2f(startX + i*100.f, winH / 2.f - 50.f);
        n.position = sf::Vector2f(startX + i*100.f, -50.f);
        nodes.push_back(n);
    }
}

void LinkedList::insert(int value, int index) {
    if (index < 0) index = 0;
    if (index > (int)nodes.size()) index = (int)nodes.size();
    Node n; n.value = value;
    n.position = sf::Vector2f(250.f + (winW - 250.f) / 2.f, winH + 100.f);
    nodes.insert(nodes.begin() + index, n);
    float totalWidth = nodes.size() * 100.f;
    float startX = 250.f + ((winW - 250.f - totalWidth) / 2.f);
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
    }
}

void LinkedList::remove(int index) {
    if (index < 0 || index >= (int)nodes.size()) return;
    nodes.erase(nodes.begin() + index);
    float totalWidth = nodes.size() * 100.f;
    float startX = 250.f + ((winW - 250.f - totalWidth) / 2.f);
    for (size_t i = 0; i < nodes.size(); ++i) {
        nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
    }
}

void LinkedList::beginInsertSteps(int value, int idx) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;
    if (idx < 0) idx = 0;
    if (idx > (int)nodes.size()) idx = (int)nodes.size();

    if (!isStepByStep) {
        insert(value, idx);
        VisualStep s; s.message = "Inserted " + std::to_string(value) + " at index " + std::to_string(idx) + " (Run at Once)";
        s.highlightColor = sf::Color(0, 200, 80);
        if (idx < (int)nodes.size()) s.highlighted = {idx};
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    for (int i = 0; i < idx && i < (int)nodes.size(); ++i) {
        VisualStep s; s.highlighted = {i};
        s.message = "Traversing to index " + std::to_string(idx) + "→ at node[" + std::to_string(i) + "] = " + std::to_string(nodes[i].value);
        animSteps.push_back(s);
    }
    VisualStep fin; fin.highlighted = {idx};
    fin.message = "Will insert " + std::to_string(value) + " at index " + std::to_string(idx);
    fin.highlightColor = sf::Color(0, 200, 80);
    animSteps.push_back(fin);

    VisualStep done; done.highlighted = {idx};
    done.message = "Inserted " + std::to_string(value) + " at index " + std::to_string(idx) + "!";
    done.highlightColor = sf::Color(0, 200, 80);
    animSteps.push_back(done);

    commitOp = [this, value, idx]() { insert(value, idx); };
    animStep = 0;
    isPlaying = true;
}

void LinkedList::beginDeleteSteps(int idx) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;
    if (idx < 0 || idx >= (int)nodes.size()) return;
    int value = nodes[idx].value;

    if (!isStepByStep) {
        remove(idx);
        VisualStep s; s.message = "Deleted node at index " + std::to_string(idx) + " with value " + std::to_string(value) + " (Run at Once)";
        s.highlightColor = sf::Color(220, 60, 60);
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    for (int i = 0; i <= idx && i < (int)nodes.size(); ++i) {
        VisualStep s; s.highlighted = {i};
        if (i < idx) s.message = "Searching for index " + std::to_string(idx) + " — at node[" + std::to_string(i) + "] = " + std::to_string(nodes[i].value);
        else { s.message = "Found node[" + std::to_string(idx) + "] = " + std::to_string(value) + ". Removing..."; s.highlightColor = sf::Color(220, 60, 60); }
        animSteps.push_back(s);
    }

    VisualStep done;
    done.message = "Deleted node at index " + std::to_string(idx) + "!";
    done.highlightColor = sf::Color(220, 60, 60);
    animSteps.push_back(done);

    commitOp = [this, idx]() { remove(idx); };
    animStep = 0;
    isPlaying = true;
}

void LinkedList::beginSearchSteps(int value) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;
    bool found = false;
    int foundIdx = -1;

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
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    for (int i = 0; i < (int)nodes.size(); ++i) {
        VisualStep s; s.highlighted = {i};
        if (nodes[i].value == value) {
            s.message = "Found " + std::to_string(value) + " at index " + std::to_string(i) + "!";
            s.highlightColor = sf::Color(0, 200, 80);
            animSteps.push_back(s);
            found = true;
            foundIdx = i;
            break;
        } else {
            s.message = "Searching for " + std::to_string(value) + " — current node is " + std::to_string(nodes[i].value);
            animSteps.push_back(s);
        }
    }

    VisualStep done;
    if (found) {
        done.message = "Search finished! Found at index " + std::to_string(foundIdx);
        done.highlightColor = sf::Color(0, 200, 80);
        done.highlighted = {foundIdx};
    } else {
        VisualStep s; s.message = std::to_string(value) + " not found in the list.";
        s.highlightColor = sf::Color(220, 60, 60);
        animSteps.push_back(s);
        done.message = "Search finished! Not found.";
        done.highlightColor = sf::Color(220, 60, 60);
    }
    animSteps.push_back(done);

    animStep = 0;
    isPlaying = true;
}

void LinkedList::play()  { isPlaying = true; playTimer = 0.f; }
void LinkedList::pause() { isPlaying = false; }

void LinkedList::stepForward() {
    if (animStep < 0 || animSteps.empty()) return;
    if (animStep + 1 < (int)animSteps.size()) {
        animStep++;
        if (animStep == (int)animSteps.size() - 1) {
            if (commitOp) { commitOp(); commitOp = nullptr; }
            isPlaying = false;
        }
    } else {
        isPlaying = false;
    }
}

void LinkedList::stepBackward() { if (animStep > 0) animStep--; }

void LinkedList::update(float dt) {
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

void LinkedList::draw(sf::RenderWindow& window) {
    sf::Text title; title.setFont(font); title.setString("Singly Linked List");
    title.setCharacterSize(24); title.setFillColor(sf::Color::White); title.setPosition(300, 10);
    window.draw(title);

    if (animStep >= 0 && animStep < (int)animSteps.size()) {
        sf::Text msg; msg.setFont(font);
        msg.setString(sf::String::fromUtf8(animSteps[animStep].message.begin(), animSteps[animStep].message.end()));
        msg.setCharacterSize(17); msg.setFillColor(sf::Color(200, 230, 255)); msg.setPosition(300, 40);
        window.draw(msg);
        sf::Text sc; sc.setFont(font);
        sc.setString("Step " + std::to_string(animStep + 1) + "/" + std::to_string((int)animSteps.size()));
        sc.setCharacterSize(14); sc.setFillColor(sf::Color(160, 160, 160)); sc.setPosition(300, 60);
        window.draw(sc);
    }

    float radius = 25.f;
    for (size_t i = 0; i < nodes.size(); ++i) {
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

void LinkedList::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    for (auto& b : buttons) b.handleEvent(event, window);
    for (auto& t : textInputs) t.handleEvent(event, window);
}

void LinkedList::updateNode(int index, int newValue) {}

void LinkedList::onResize(float w, float h) {
    winW = w; winH = h;
    initUI();
    if (!nodes.empty()) {
        float totalWidth = nodes.size() * 100.f;
        float startX = 250.f + ((winW - 250.f - totalWidth) / 2.f);
        for (size_t i = 0; i < nodes.size(); ++i) {
            nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
        }
    }
}
