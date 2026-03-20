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
    
    float uiY1 = winH - 160.f;
    float uiY2 = winH - 120.f;
    
    textInputs.push_back(TextInput(sf::Vector2f(120,30), sf::Vector2f(300, uiY1), font, "e.g. 10,20,30"));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(300, uiY2), "Init Array", font, [this]() {
        std::string t = textInputs[0].getText(); std::vector<int> res; size_t pos = 0;
        while ((pos = t.find(',')) != std::string::npos) { if (pos>0) res.push_back(std::stoi(t.substr(0,pos))); t.erase(0,pos+1); }
        if (!t.empty()) res.push_back(std::stoi(t));
        if (!res.empty()) init(res);
    }));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(430, uiY2), "Init Random", font, [this]() {
        std::vector<int> r; for (int i=0;i<15;++i) r.push_back(rand()%100); init(r);
    }));
    textInputs.push_back(TextInput(sf::Vector2f(60,30), sf::Vector2f(560, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(560, uiY2), "Insert", font, [this]() {
        std::string v = textInputs[1].getText(); if (!v.empty()) beginInsertSteps(std::stoi(v));
    }));
    buttons.push_back(Button(sf::Vector2f(110,30), sf::Vector2f(690, uiY2), "Insert Random", font, [this]() {
        beginInsertSteps(rand()%100);
    }));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(810, uiY2), "Extract Root", font, [this]() {
        beginExtractSteps();
    }));
    textInputs.push_back(TextInput(sf::Vector2f(60,30), sf::Vector2f(940, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90,30), sf::Vector2f(940, uiY2), "Search", font, [this]() {
        std::string v = textInputs[2].getText(); if (!v.empty()) beginSearchSteps(std::stoi(v));
    }));
    buttons.push_back(Button(sf::Vector2f(130,30), sf::Vector2f(1040, uiY2), "Toggle Min/Max", font, [this]() {
        isMinHeap = !isMinHeap; if (!rawData.empty()) buildHeap();
    }));
}

void MinMaxHeap::beginInsertSteps(int value) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;

    if (!isStepByStep) {
        insert(value);
        VisualStep s; s.message = "Inserted " + std::to_string(value) + " (Run at Once)";
        s.highlightColor = sf::Color(0, 200, 80);
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    rawData.push_back(value);
    Node n; n.value = value;
    
    float rootX = 250.f + ((winW - 250.f) / 2.f);
    n.position = sf::Vector2f(rootX, winH + 100.f); 
    nodes.push_back(n);
    recalculateTargetPositions();
    int idx = (int)rawData.size() - 1;

    { VisualStep s; s.highlighted = {idx};
      s.message = "Placing " + std::to_string(value) + " at position [" + std::to_string(idx) + "]";
      s.highlightColor = sf::Color(80,200,255); animSteps.push_back(s); }

    std::vector<int> sim = rawData;
    int si = idx;
    while (si > 0) {
        int parent = (si - 1) / 2;
        bool doSwap = isMinHeap ? sim[si] < sim[parent] : sim[si] > sim[parent];
        VisualStep s; s.highlighted = {si, parent};
        if (doSwap) {
            s.message = "HeapifyUp: ["+std::to_string(si)+"]="+std::to_string(sim[si])+" < parent ["+std::to_string(parent)+"]="+std::to_string(sim[parent])+" → Swap";
            animSteps.push_back(s);
            std::swap(sim[si], sim[parent]); si = parent;
        } else {
            s.message = "HeapifyUp: ["+std::to_string(si)+"]="+std::to_string(sim[si])+" OK vs parent ["+std::to_string(parent)+"]="+std::to_string(sim[parent])+" → Done";
            s.highlightColor = sf::Color(0,200,80); animSteps.push_back(s); break;
        }
    }


    VisualStep done; done.message = "Inserted " + std::to_string(value) + "!";
    done.highlightColor = sf::Color(0, 200, 80);
    animSteps.push_back(done);

    int insertedIdx = idx;
    commitOp = [this, insertedIdx]() { heapifyUp(insertedIdx); recalculateTargetPositions(); };
    animStep = 0;
    isPlaying = true;
}

void MinMaxHeap::beginExtractSteps() {
    if (rawData.empty()) return;
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;

    if (!isStepByStep) {
        extract();
        VisualStep s; s.message = "Extracted root (Run at Once)";
        s.highlightColor = sf::Color(220, 60, 60);
        animSteps.push_back(s);
        animStep = 0;
        return;
    }

    int rootVal = rawData[0];
    int lastIdx = (int)rawData.size() - 1;

    { VisualStep s; s.highlighted = {0};
      s.message = "Extracting root: "+std::to_string(rootVal);
      s.highlightColor = sf::Color(220,60,60); animSteps.push_back(s); }

    if (lastIdx > 0) {
        VisualStep s; s.highlighted = {0, lastIdx};
        s.message = "Moving last element ["+std::to_string(rawData[lastIdx])+"] to root position";
        animSteps.push_back(s);
    }

    std::vector<int> sim = rawData;
    sim[0] = sim.back(); sim.pop_back();
    int n = (int)sim.size(), si = 0;
    while (true) {
        int left=2*si+1, right=2*si+2, target=si;
        if (left<n && (isMinHeap ? sim[left]<sim[target] : sim[left]>sim[target])) target=left;
        if (right<n && (isMinHeap ? sim[right]<sim[target] : sim[right]>sim[target])) target=right;
        if (target != si) {
            VisualStep s; s.highlighted = {si, target};
            s.message = "HeapifyDown: ["+std::to_string(si)+"]="+std::to_string(sim[si])+" → swap with ["+std::to_string(target)+"]="+std::to_string(sim[target]);
            animSteps.push_back(s);
            std::swap(sim[si], sim[target]); si = target;
        } else {
            VisualStep s; s.highlighted = {si};
            s.message = "Heap property satisfied. Done!";
            s.highlightColor = sf::Color(0,200,80); animSteps.push_back(s); break;
        }
    }
    
    VisualStep done; done.message = "Extracted root!";
    done.highlightColor = sf::Color(220, 60, 60);
    animSteps.push_back(done);

    commitOp = [this]() { extract(); };
    animStep = 0;
    isPlaying = true;
}

