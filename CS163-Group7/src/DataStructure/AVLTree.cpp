#include "DataStructure/AVLTree.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

AVLTree::AVLTree(float windowWidth, float windowHeight) : root(nullptr) {
    winW = windowWidth;
    winH = windowHeight;
    if (!font.loadFromFile("assets/fonts/arial.ttf")) std::cerr << "Failed to load font\n";
    initUI();
}
AVLTree::~AVLTree() { clearAnimSteps(); deleteTree(root); }

void AVLTree::deleteTree(TreeNode* n) {
    if (!n) return; deleteTree(n->left); deleteTree(n->right); 
    // Do NOT delete if it might be in nodeCache? Actually deleteTree is for real root.
    // We should probably clear nodeCache too.
    delete n;
}

void AVLTree::clearAnimSteps() {
    for (auto& s : animSteps) { if (s.treeSnapshot) deleteSimTree(s.treeSnapshot); }
    animSteps.clear();
}

void AVLTree::initUI() {
    buttons.clear();
    textInputs.clear();
    
    float visualAreaCenter = 200.f + (winW - 200.f - codePaneWidth) / 2.f;
    float uiY1 = winH - 100.f; // Row 1 Inputs
    float uiY2 = winH - 60.f;  // Row 1 Buttons
    
    // Total width roughly 750px
    float startX = visualAreaCenter - 375.f;
    if (startX < 210.f) startX = 210.f;

    textInputs.push_back(TextInput(sf::Vector2f(120,30), sf::Vector2f(startX, uiY1), font, "e.g. 10,20,30"));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(startX, uiY2), "Init Array", font, [this]() {
        std::string t = textInputs[0].getText(); std::vector<int> res; size_t pos=0;
        while ((pos=t.find(','))!=std::string::npos){if(pos>0)res.push_back(std::stoi(t.substr(0,pos)));t.erase(0,pos+1);}
        if (!t.empty()) res.push_back(std::stoi(t)); if (!res.empty()) init(res);
    }));

    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(startX + 130, uiY2), "Init Random", font, [this]() {
        std::vector<int> r; for (int i=0;i<10;++i) r.push_back(rand()%100); init(r);
    }));

    textInputs.push_back(TextInput(sf::Vector2f(90,30), sf::Vector2f(startX + 260, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90,30), sf::Vector2f(startX + 260, uiY2), "Insert", font, [this]() {
        std::string v = textInputs[1].getText(); if (!v.empty()) beginInsertSteps(std::stoi(v));
    }));

    buttons.push_back(Button(sf::Vector2f(110,30), sf::Vector2f(startX + 360, uiY2), "Add Random", font, [this]() {
        beginInsertSteps(rand()%100);
    }));

    textInputs.push_back(TextInput(sf::Vector2f(90,30), sf::Vector2f(startX + 480, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90,30), sf::Vector2f(startX + 480, uiY2), "Delete", font, [this]() {
        std::string v = textInputs[2].getText(); if (!v.empty()) beginDeleteSteps(std::stoi(v));
    }));

    textInputs.push_back(TextInput(sf::Vector2f(90,30), sf::Vector2f(startX + 580, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90,30), sf::Vector2f(startX + 580, uiY2), "Search", font, [this]() {
        std::string v = textInputs[3].getText(); if (!v.empty()) beginSearchSteps(std::stoi(v));
    }));
}

AVLTree::SimNode* AVLTree::copySimTree(SimNode* node) {
    if (!node) return nullptr;
    return new SimNode{node->value, node->height, copySimTree(node->left), copySimTree(node->right)};
}

void AVLTree::deleteSimTree(SimNode* node) {
    if (!node) return;
    deleteSimTree(node->left); deleteSimTree(node->right); delete node;
}

int AVLTree::getSimHeight(SimNode* n) { return n ? n->height : 0; }
int AVLTree::getSimBalance(SimNode* n) { return n ? getSimHeight(n->left) - getSimHeight(n->right) : 0; }

AVLTree::SimNode* AVLTree::rotateRightSim(SimNode* y) {
    if (!y || !y->left) return y;
    SimNode* x = y->left;
    SimNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = (std::max)(getSimHeight(y->left), getSimHeight(y->right)) + 1;
    x->height = (std::max)(getSimHeight(x->left), getSimHeight(x->right)) + 1;
    return x;
}

AVLTree::SimNode* AVLTree::rotateLeftSim(SimNode* x) {
    if (!x || !x->right) return x;
    SimNode* y = x->right;
    SimNode* T2 = y->left;

    y->left = x;
    x->right = T2;
    
    x->height = (std::max)(getSimHeight(x->left), getSimHeight(x->right)) + 1;
    y->height = (std::max)(getSimHeight(y->left), getSimHeight(y->right)) + 1;
    return y;
}

void AVLTree::insertNodeSim(SimNode** nodeRef, int value, std::vector<VisualStep>& steps, SimNode** rootRef) {
    auto takeSnapshot = [&](const std::string& msg, const std::vector<int>& hl, sf::Color clr, int pivot = -1, int unbal = -1, int line = -1) {
        VisualStep s; s.message = msg; s.highlightedValues = hl; s.highlightColor = clr;
        s.pivotValue = pivot; s.unbalancedValue = unbal;
        s.treeSnapshot = copySimTree(*rootRef);
        s.codeLine = line;
        steps.push_back(s);
    };

    SimNode* node = *nodeRef;
    takeSnapshot("Checking if node is nullptr", {}, sf::Color(220, 180, 0), -1, -1, 1);
    
    if (!node) {
        *nodeRef = new SimNode{value, 1, nullptr, nullptr};
        takeSnapshot("Node is nullptr: Creating new leaf node " + std::to_string(value), {value}, sf::Color(0, 200, 80), -1, -1, 2);
        return;
    }

    takeSnapshot("Checking if " + std::to_string(value) + " < " + std::to_string(node->value), {node->value}, sf::Color(220, 180, 0), -1, -1, 4);
    
    if (value < node->value) {
        insertNodeSim(&(node->left), value, steps, rootRef);
    } else {
        takeSnapshot("Checking if " + std::to_string(value) + " > " + std::to_string(node->value), {node->value}, sf::Color(220, 180, 0), -1, -1, 6);
        if (value > node->value) {
            insertNodeSim(&(node->right), value, steps, rootRef);
        } else {
            takeSnapshot(std::to_string(value) + " already exists!", {node->value}, sf::Color(255, 60, 60), -1, -1, 9);
            return;
        }
    }

    node = *nodeRef;
    node->height = 1 + std::max(getSimHeight(node->left), getSimHeight(node->right));
    takeSnapshot("Updating height for " + std::to_string(node->value), {node->value}, sf::Color(0, 150, 255), -1, -1, 11);

    int bal = getSimBalance(node);
    takeSnapshot("Checking balance of " + std::to_string(node->value), {node->value}, sf::Color(220, 180, 0), -1, -1, 12);

    // LL
    takeSnapshot("Checking LL case...", {node->value}, sf::Color(220, 180, 0), -1, -1, 13);
    if (bal > 1 && value < (node->left ? node->left->value : -1e9)) {
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("Performing Right Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 14);
        return;
    }
    // RR
    takeSnapshot("Checking RR case...", {node->value}, sf::Color(220, 180, 0), -1, -1, 16);
    if (bal < -1 && value > (node->right ? node->right->value : 1e9)) {
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("Performing Left Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 17);
        return;
    }
    // LR
    takeSnapshot("Checking LR case...", {node->value}, sf::Color(220, 180, 0), -1, -1, 19);
    if (bal > 1 && value > (node->left ? node->left->value : 1e9)) {
        int childVal = node->left->value;
        node->left = rotateLeftSim(node->left);
        takeSnapshot("LR Case: Double Rotation (Part 1 - Rotate Child Left)", {node->left->value}, sf::Color(255, 150, 100), -1, -1, 20);
        
        node = *nodeRef; // Refresh after child rotation
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("LR Case: Double Rotation (Part 2 - Rotate Node Right)", {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 21);
        return;
    }
    // RL
    takeSnapshot("Checking RL case...", {node->value}, sf::Color(220, 180, 0), -1, -1, 23);
    if (bal < -1 && value < (node->right ? node->right->value : -1e9)) {
        int childVal = node->right->value;
        node->right = rotateRightSim(node->right);
        takeSnapshot("RL Case: Double Rotation (Part 1 - Rotate Child Right)", {node->right->value}, sf::Color(255, 150, 100), -1, -1, 24);
        
        node = *nodeRef; // Refresh after child rotation
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("RL Case: Double Rotation (Part 2 - Rotate Node Left)", {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 25);
        return;
    }
    
    // Default return node
    takeSnapshot("Node " + std::to_string(node->value) + " is balanced.", {node->value}, sf::Color(0, 150, 255), -1, -1, 27);
}

void AVLTree::removeNodeSim(SimNode** nodeRef, int value, std::vector<VisualStep>& steps, SimNode** rootRef) {
    auto takeSnapshot = [&](const std::string& msg, const std::vector<int>& hl, sf::Color clr, int pivot = -1, int unbal = -1, int line = -1) {
        VisualStep s; s.message = msg; s.highlightedValues = hl; s.highlightColor = clr;
        s.pivotValue = pivot; s.unbalancedValue = unbal;
        s.treeSnapshot = copySimTree(*rootRef);
        s.codeLine = line;
        steps.push_back(s);
    };

    SimNode* node = *nodeRef;
    takeSnapshot("Checking if node is nullptr", {}, sf::Color(220, 180, 0), -1, -1, 1);
    if (!node) return;

    takeSnapshot("Checking if " + std::to_string(value) + " < " + std::to_string(node->value), {node->value}, sf::Color(220, 180, 0), -1, -1, 4);
    if (value < node->value) {
        removeNodeSim(&(node->left), value, steps, rootRef);
    } else {
        takeSnapshot("Checking if " + std::to_string(value) + " > " + std::to_string(node->value), {node->value}, sf::Color(220, 180, 0), -1, -1, 6);
        if (value > node->value) {
            removeNodeSim(&(node->right), value, steps, rootRef);
        } else {
            // Found node to delete
            takeSnapshot("Found " + std::to_string(value) + "! Preparing to remove...", {node->value}, sf::Color(255, 60, 60), -1, -1, 8);

            takeSnapshot("Checking number of children...", {node->value}, sf::Color(220, 180, 0), -1, -1, 9);
            if (!node->left || !node->right) {
                SimNode* t = node->left ? node->left : node->right;
                takeSnapshot("Single child or leaf case. Examining child...", {node->value}, sf::Color(220, 180, 0), -1, -1, 10);
                if (!t) {
                    takeSnapshot("Leaf case: Removing node.", {node->value}, sf::Color(255, 60, 60), -1, -1, 11);
                    *nodeRef = nullptr; delete node; return; 
                } else { 
                    SimNode* res = new SimNode{*t}; 
                    *nodeRef = res; delete node; 
                    takeSnapshot("Replacing node with its only child.", {res->value}, sf::Color(200, 100, 255), -1, -1, 14);
                    return; 
                }
            } else {
                takeSnapshot("Two children case: Finding successor...", {node->value}, sf::Color(220, 180, 0), -1, -1, 17);
                SimNode* successor = minValueNodeSim(node->right);
                int succVal = successor->value;
                node->value = succVal;
                takeSnapshot("Successor is " + std::to_string(succVal) + ". Replacing value.", {node->value}, sf::Color(0, 200, 80), -1, -1, 18);
                removeNodeSim(&(node->right), succVal, steps, rootRef);
                // node pointer was updated by its right child removal
                node = *nodeRef;
            }
        }
    }

    node = *nodeRef;
    if (!node) {
        // Highlight line 18: if (node == nullptr) return nullptr;
        // steps.push_back(...) is tricky here since we already returned in base cases
        return; 
    }
    
    takeSnapshot("Checking if node is nullptr after deletion...", {node->value}, sf::Color(220, 180, 0), -1, -1, 22);

    node->height = 1 + std::max(getSimHeight(node->left), getSimHeight(node->right));
    takeSnapshot("Updating height and checking balance...", {node->value}, sf::Color(0, 150, 255), -1, -1, 25);

    int bal = getSimBalance(node);
    takeSnapshot("Checking balance (bf=" + std::to_string(bal) + ")", {node->value}, sf::Color(220, 180, 0), -1, -1, 26);

    if (bal > 1 && getSimBalance(node->left) >= 0) {
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("Right Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 28);
    } else if (bal > 1 && getSimBalance(node->left) < 0) {
        int childVal = node->left->value;
        node->left = rotateLeftSim(node->left);
        takeSnapshot("Double Rotation: Rotating left on child " + std::to_string(childVal), {node->left->value}, sf::Color(255, 150, 100), -1, -1, 31);
        
        node = *nodeRef;
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("Double Rotation: Now rotating right on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 32);
    } else if (bal < -1 && getSimBalance(node->right) <= 0) {
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("Left Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 35);
    } else if (bal < -1 && getSimBalance(node->right) > 0) {
        int childVal = node->right->value;
        node->right = rotateRightSim(node->right);
        takeSnapshot("Double Rotation: Rotating right on child " + std::to_string(childVal), {node->right->value}, sf::Color(255, 150, 100), -1, -1, 38);
        
        node = *nodeRef;
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("Double Rotation: Now rotating left on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal, 39);
    }
}

AVLTree::SimNode* AVLTree::minValueNodeSim(SimNode* node) {
    SimNode* cur = node; while (cur->left) cur = cur->left; return cur;
}

// ---- applySimStructure ----
void AVLTree::applySimStructure(SimNode* simRoot) {
    std::set<TreeNode*> usedNodes;
    auto sync = [&](auto self, SimNode* sn) -> TreeNode* {
        if (!sn) return nullptr;
        TreeNode* tn = nullptr;
        if (nodeCache.count(sn->value)) {
            tn = nodeCache[sn->value];
            // Cycle/Duplicate prevention: if this cache node is already in use in this frame,
            // (happens during transient states in deletion), create a temp node.
            if (usedNodes.count(tn)) {
                tn = new TreeNode(sn->value);
                // Position it near the original for smooth move
                tn->position = tn->targetPosition = nodeCache[sn->value]->position;
            } else {
                usedNodes.insert(tn);
            }
        } else {
            tn = new TreeNode(sn->value);
            float rootX = 200.f + ((winW - 200.f - codePaneWidth) / 2.f);
            tn->position = tn->targetPosition = sf::Vector2f(rootX, 50.f);
            nodeCache[sn->value] = tn;
            usedNodes.insert(tn);
        }
        tn->height = sn->height;
        tn->left = self(self, sn->left);
        tn->right = self(self, sn->right);
        return tn;
    };
    root = sync(sync, simRoot);
}

// ---- Step helpers ----
void AVLTree::beginInsertSteps(int value) {
    clearAnimSteps(); commitOp = nullptr; isPlaying = false; playTimer = 0.f; nodeCache.clear();
    
    currentCode = {
        "TreeNode* insert(TreeNode* node, int value) {",  // 0
        "  if (node == nullptr) {",                      // 1
        "    return new TreeNode(value);",               // 2
        "  }",                                           // 3
        "  if (value < node->value) {",                  // 4
        "    node->left = insert(node->left, value);",   // 5
        "  } else if (value > node->value) {",           // 6
        "    node->right = insert(node->right, value);",  // 7
        "  }",                                           // 8
        "  node->height = 1 + max(getHeight(node->left), getHeight(node->right));", // 9
        "  int balance = getBalance(node);",             // 10
        "  if (balance > 1 && value < node->left->value) {", // 11
        "    return rotateRight(node);",                 // 12
        "  }",                                           // 13
        "  if (balance < -1 && value > node->right->value) {", // 14
        "    return rotateLeft(node);",                  // 15
        "  }",                                           // 16
        "  if (balance > 1 && value > node->left->value) {", // 17
        "    node->left = rotateLeft(node->left);",      // 18
        "    return rotateRight(node);",                 // 19
        "  }",                                           // 20
        "  if (balance < -1 && value < node->right->value) {", // 21
        "    node->right = rotateRight(node->right);",   // 22
        "    return rotateLeft(node);",                  // 23
        "  }",                                           // 24
        "  return node;",                                // 25
        "}"                                              // 26
    };
    // Build initial cache from current root
    auto populate = [&](auto self, TreeNode* n) -> void {
        if (!n) return; nodeCache[n->value] = n;
        self(self, n->left); self(self, n->right);
    };
    populate(populate, root);

    if (!isStepByStep) {
        bool exists = false; TreeNode* c = root;
        while(c) { if (value < c->value) c=c->left; else if (value>c->value) c=c->right; else { exists = true; break; } }
        if (!exists) insert(value);
        VisualStep s; 
        s.message = exists ? std::to_string(value) + " already exists in tree (Run at Once)" : "Inserted " + std::to_string(value) + " (Run at Once)";
        s.highlightColor = exists ? sf::Color(220, 60, 60) : sf::Color(0, 200, 80);
        animSteps.push_back(s); animStep = 0; return;
    }

    SimNode* simRoot = nullptr;
    auto toSim = [&](auto self, TreeNode* n) -> SimNode* {
        if (!n) return nullptr;
        return new SimNode{n->value, n->height, self(self, n->left), self(self, n->right)};
    };
    simRoot = toSim(toSim, root);
    insertNodeSim(&simRoot, value, animSteps, &simRoot);
    deleteSimTree(simRoot);

    if (animSteps.empty()) {
         VisualStep s; s.message = std::to_string(value) + " already exists!";
         s.highlightColor = sf::Color(220,60,60); animSteps.push_back(s);
    } else {
        VisualStep done; done.message = "Operation complete: " + std::to_string(value) + " processed!";
        done.highlightColor = sf::Color(0, 200, 80);
        done.treeSnapshot = copySimTree(animSteps.back().treeSnapshot);
        animSteps.push_back(done);
        commitOp = [this, value]() { insert(value); };
    }
    animStep = 0;
    if (!animSteps.empty() && animSteps[animStep].treeSnapshot) applySimStructure(animSteps[animStep].treeSnapshot);
    isPlaying = true;
}

void AVLTree::beginDeleteSteps(int value) {
    clearAnimSteps(); commitOp = nullptr; isPlaying = false; playTimer = 0.f; nodeCache.clear();
    
    currentCode = {
        "TreeNode* remove(TreeNode* node, int value) {",  // 0
        "  if (node == nullptr) {",                      // 1
        "    return nullptr;",                            // 2
        "  }",                                           // 3
        "  if (value < node->value) {",                  // 4
        "    node->left = remove(node->left, value);",    // 5
        "  } else if (value > node->value) {",           // 6
        "    node->right = remove(node->right, value);",   // 7
        "  } else {",                                     // 8
        "    if (node->left == nullptr || node->right == nullptr) {", // 9
        "      TreeNode* temp = node->left ? node->left : node->right;", // 10
        "      if (temp == nullptr) {",                   // 11
        "        temp = node; node = nullptr;",           // 12
        "      } else {",                                 // 13
        "        *node = *temp; delete temp;",            // 14
        "      }",                                        // 15
        "    } else {",                                   // 16
        "      TreeNode* temp = minValueNode(node->right);", // 17
        "      node->value = temp->value;",               // 18
        "      node->right = remove(node->right, temp->value);", // 19
        "    }",                                          // 20
        "  }",                                            // 21
        "  if (node == nullptr) return nullptr;",         // 22
        "  node->height = 1 + max(getHeight(node->left), getHeight(node->right));", // 23
        "  int bal = getBalance(node);",                  // 24
        "  if (bal > 1 && getBalance(node->left) >= 0) {", // 25
        "    return rotateRight(node);",                  // 26
        "  }",                                            // 27
        "  if (bal > 1 && getBalance(node->left) < 0) {", // 28
        "    node->left = rotateLeft(node->left);",       // 29
        "    return rotateRight(node);",                  // 30
        "  }",                                            // 31
        "  if (bal < -1 && getBalance(node->right) <= 0) {", // 32
        "    return rotateLeft(node);",                   // 33
        "  }",                                            // 34
        "  if (bal < -1 && getBalance(node->right) > 0) {", // 35
        "    node->right = rotateRight(node->right);",    // 36
        "    return rotateLeft(node);",                   // 37
        "  }",                                            // 38
        "  return node;",                                 // 39
        "}"                                               // 40
    };
    auto populate = [&](auto self, TreeNode* n) -> void {
        if (!n) return; nodeCache[n->value] = n;
        self(self, n->left); self(self, n->right);
    };
    populate(populate, root);

    if (!isStepByStep) {
        remove(value);
        VisualStep s; s.message = "Deleted " + std::to_string(value) + " (Run at Once)";
        s.highlightColor = sf::Color(220, 60, 60);
        animSteps.push_back(s); animStep = 0; return;
    }

    SimNode* simRoot = nullptr;
    auto toSim = [&](auto self, TreeNode* n) -> SimNode* {
        if (!n) return nullptr;
        return new SimNode{n->value, n->height, self(self, n->left), self(self, n->right)};
    };
    simRoot = toSim(toSim, root);
    removeNodeSim(&simRoot, value, animSteps, &simRoot);
    deleteSimTree(simRoot);

    if (animSteps.empty()) {
        VisualStep s; s.message = std::to_string(value) + " not found.";
        s.highlightColor = sf::Color(220,60,60); animSteps.push_back(s);
    } else {
        VisualStep done; done.message = "Operation complete: " + std::to_string(value) + " removed!";
        done.highlightColor = sf::Color(220, 60, 60);
        done.treeSnapshot = animSteps.back().treeSnapshot ? copySimTree(animSteps.back().treeSnapshot) : nullptr;
        animSteps.push_back(done);
        commitOp = [this, value]() { remove(value); };
    }
    animStep = 0;
    if (!animSteps.empty() && animSteps[animStep].treeSnapshot) applySimStructure(animSteps[animStep].treeSnapshot);
    isPlaying = true;
}

void AVLTree::beginSearchSteps(int value) {
    animSteps.clear(); commitOp = nullptr; isPlaying = false; playTimer = 0.f;
    
    currentCode = {
        "bool search(TreeNode* node, int value) {",  // 0
        "  if (node == nullptr) {",                   // 1
        "    return false;",                          // 2
        "  }",                                        // 3
        "  if (node->value == value) {",              // 4
        "    return true;",                           // 5
        "  }",                                        // 6
        "  if (value < node->value) {",               // 7
        "    return search(node->left, value);",      // 8
        "  }",                                        // 9
        "  return search(node->right, value);",       // 10
        "}"                                           // 11
    };

    if (!isStepByStep) {
        bool f = false; TreeNode* c = root;
        while(c) { if (value < c->value) c=c->left; else if (value>c->value) c=c->right; else { f=true; break; } }
        VisualStep s;
        if(f) {
            s.message = "Found " + std::to_string(value) + " (Run at Once)";
            s.highlightColor = sf::Color(0, 200, 80); s.highlightedValues = {value};
        } else {
            s.message = std::to_string(value) + " not found (Run at Once)";
            s.highlightColor = sf::Color(220, 60, 60);
        }
        animSteps.push_back(s); animStep = 0; return;
    }

    TreeNode* curr = root;
    bool found = false;
    while (curr) {
        VisualStep s_check; s_check.highlightedValues = {curr->value};
        s_check.message = "Checking if " + std::to_string(value) + " == " + std::to_string(curr->value);
        s_check.codeLine = 4; // if (node->value == value)
        animSteps.push_back(s_check);

        if (value == curr->value) {
            VisualStep s_found; s_found.highlightedValues = {curr->value};
            s_found.message = "Found " + std::to_string(value) + "!";
            s_found.highlightColor = sf::Color(0, 200, 80);
            s_found.codeLine = 5;
            animSteps.push_back(s_found);
            found = true; break;
        }

        VisualStep s_comp; s_comp.highlightedValues = {curr->value};
        s_comp.message = "Checking if " + std::to_string(value) + " < " + std::to_string(curr->value);
        s_comp.codeLine = 7; // if (value < node->value)
        animSteps.push_back(s_comp);

        if (value < curr->value) {
            VisualStep s_left; s_left.highlightedValues = {curr->value};
            s_left.message = std::to_string(value) + " < " + std::to_string(curr->value) + " -> Left";
            s_left.codeLine = 8;
            animSteps.push_back(s_left);
            curr = curr->left;
        } else {
            VisualStep s_right; s_right.highlightedValues = {curr->value};
            s_right.message = std::to_string(value) + " > " + std::to_string(curr->value) + " -> Right";
            s_right.codeLine = 10;
            animSteps.push_back(s_right);
            curr = curr->right;
        }
    }

    if (!found) {
        VisualStep s_null; s_null.message = "Leaf reached, " + std::to_string(value) + " not found.";
        s_null.codeLine = 1;
        animSteps.push_back(s_null);
        
        VisualStep s_fail; s_fail.message = "Returning false.";
        s_fail.codeLine = 2;
        animSteps.push_back(s_fail);
    }
    
    VisualStep done;
    if (found) {
        done.message = "Search finished! Found " + std::to_string(value);
        done.highlightColor = sf::Color(0, 200, 80);
        done.highlightedValues = {value};
    } else {
        VisualStep s; s.message = std::to_string(value)+" not found in tree.";
        s.highlightColor = sf::Color(200,80,80); animSteps.push_back(s);
        done.message = "Search finished! Not found.";
        done.highlightColor = sf::Color(220, 60, 60);
    }
    animSteps.push_back(done);

    animStep = 0;
    isPlaying = true;
}

std::vector<std::string> AVLTree::getCode() const { return currentCode; }
int AVLTree::getCurrentLine() const { return (animStep >= 0 && animStep < (int)animSteps.size()) ? animSteps[animStep].codeLine : -1; }

// ---- play/pause/step ----
void AVLTree::play()  { isPlaying = true; playTimer = 0.f; }
void AVLTree::pause() { isPlaying = false; }
void AVLTree::stepForward() {
    if (animStep < 0 || animSteps.empty()) return;
    if (animStep + 1 < (int)animSteps.size()) {
        animStep++;
        if (animSteps[animStep].treeSnapshot) applySimStructure(animSteps[animStep].treeSnapshot);
        if (animStep == (int)animSteps.size() - 1) { 
            if (commitOp) { commitOp(); commitOp = nullptr; }
            isPlaying = false;
        }
    } else {
        isPlaying = false;
    }
}
void AVLTree::stepBackward() { 
    if (animStep > 0) {
        animStep--; 
        if (animSteps[animStep].treeSnapshot) applySimStructure(animSteps[animStep].treeSnapshot);
    }
}

// ---- AVL ops ----
int AVLTree::getHeight(TreeNode* n) { return n ? n->height : 0; }
int AVLTree::getBalance(TreeNode* n) { return n ? getHeight(n->left)-getHeight(n->right) : 0; }

AVLTree::TreeNode* AVLTree::rotateRight(TreeNode* y) {
    TreeNode* x=y->left, *T2=x->right; x->right=y; y->left=T2;
    y->height=std::max(getHeight(y->left),getHeight(y->right))+1;
    x->height=std::max(getHeight(x->left),getHeight(x->right))+1; return x;
}
AVLTree::TreeNode* AVLTree::rotateLeft(TreeNode* x) {
    TreeNode* y=x->right, *T2=y->left; y->left=x; x->right=T2;
    x->height=std::max(getHeight(x->left),getHeight(x->right))+1;
    y->height=std::max(getHeight(y->left),getHeight(y->right))+1; return y;
}
AVLTree::TreeNode* AVLTree::insertNode(TreeNode* node, int value, sf::Vector2f startPos) {
    if (!node) {
        TreeNode* nn = new TreeNode(value);
        nn->position = startPos;
        return nn;
    }
    if (value<node->value) node->left=insertNode(node->left,value, startPos);
    else if (value>node->value) node->right=insertNode(node->right,value, startPos);
    else return node;
    node->height=1+std::max(getHeight(node->left),getHeight(node->right));
    int bal=getBalance(node);
    if (bal>1&&value<node->left->value) return rotateRight(node);
    if (bal<-1&&value>node->right->value) return rotateLeft(node);
    if (bal>1&&value>node->left->value){node->left=rotateLeft(node->left);return rotateRight(node);}
    if (bal<-1&&value<node->right->value){node->right=rotateRight(node->right);return rotateLeft(node);}
    return node;
}
AVLTree::TreeNode* AVLTree::minValueNode(TreeNode* node) {
    TreeNode* cur=node; while(cur->left)cur=cur->left; return cur;
}
AVLTree::TreeNode* AVLTree::removeNode(TreeNode* node, int value) {
    if (!node) return node;
    if (value<node->value) node->left=removeNode(node->left,value);
    else if (value>node->value) node->right=removeNode(node->right,value);
    else {
        if (!node->left||!node->right){
            TreeNode* t=node->left?node->left:node->right;
            if(!t){t=node;node=nullptr;}else *node=*t; delete t;
        } else {
            TreeNode* t=minValueNode(node->right);
            node->value=t->value; node->right=removeNode(node->right,t->value);
        }
    }
    if (!node) return node;
    node->height=1+std::max(getHeight(node->left),getHeight(node->right));
    int bal=getBalance(node);
    if (bal>1&&getBalance(node->left)>=0) return rotateRight(node);
    if (bal>1&&getBalance(node->left)<0){node->left=rotateLeft(node->left);return rotateRight(node);}
    if (bal<-1&&getBalance(node->right)<=0) return rotateLeft(node);
    if (bal<-1&&getBalance(node->right)>0){node->right=rotateRight(node->right);return rotateLeft(node);}
    return node;
}
void AVLTree::calcPositions(TreeNode* n, int depth, float hs, float vs, float startX, int& index) {
    if (!n) return;
    calcPositions(n->left, depth + 1, hs, vs, startX, index);
    n->targetPosition = {startX + index * hs, 80.f + depth * vs};
    index++;
    calcPositions(n->right, depth + 1, hs, vs, startX, index);
}

// ---- update ----
void AVLTree::update(float dt) {
    float rootX = 200.f + ((winW - 200.f - codePaneWidth) / 2.f);
    
    // Count nodes for in-order layout
    int totalNodes = 0;
    auto count = [&](auto self, TreeNode* n) -> void {
        if (!n) return;
        totalNodes++;
        self(self, n->left);
        self(self, n->right);
    };
    count(count, root);

    int h = getHeight(root);
    float baseNodeGap = 50.f;
    float baseVerticalGap = 80.f;

    float requiredWidth = totalNodes * baseNodeGap;
    float requiredHeight = h * baseVerticalGap;

    float availW = (winW - 200.f - codePaneWidth) * 0.95f;
    float availH = (winH - 150.f) * 0.95f;

    // Determine scale factor
    float scaleW = (requiredWidth > 0) ? availW / requiredWidth : 1.0f;
    float scaleH = (requiredHeight > 0) ? availH / requiredHeight : 1.0f;
    scaleFactor = std::min({1.0f, scaleW, scaleH});
    
    // Minimum scale to prevent microscopic nodes, but don't let it overflow TOO much
    if (scaleFactor < 0.2f) scaleFactor = 0.2f;

    float hs = baseNodeGap * scaleFactor;
    float vs = baseVerticalGap * scaleFactor;
    
    // If the scaled gap is too small for readable nodes, enforce a minimum
    float minRadius = 14.f;
    if (hs < minRadius * 2.2f) hs = minRadius * 2.2f;

    float totalTreeWidth = totalNodes * hs;
    float startX = 200.f + ((winW - 200.f - codePaneWidth - totalTreeWidth) / 2.f) + (hs / 2.f);

    int index = 0;
    calcPositions(root, 0, hs, vs, startX - (hs/2.f), index);

    std::function<void(TreeNode*)> anim = [&](TreeNode* n) {
        if (!n) return;
        if (std::isnan(n->position.x) || std::isnan(n->position.y)) n->position = sf::Vector2f(rootX, 50.f);
        if (std::isnan(n->targetPosition.x) || std::isnan(n->targetPosition.y)) n->targetPosition = sf::Vector2f(rootX, 50.f);
        n->position.x += (n->targetPosition.x - n->position.x)*12.f*dt;
        n->position.y += (n->targetPosition.y - n->position.y)*12.f*dt;
        anim(n->left); anim(n->right);
    };
    anim(root);
    if (isPlaying && animStep >= 0) { playTimer += dt; if (playTimer >= playInterval) { playTimer = 0.f; stepForward(); } }
}

// ---- drawNode ----
void AVLTree::drawNode(sf::RenderWindow& window, TreeNode* node,
                       const std::vector<int>& hlValues, sf::Color hlColor,
                       int pivotValue, int unbalancedValue) {
    if (!node) return;
    float radius = std::max(14.f, 22.f * scaleFactor);

    // Lines to children
    auto drawLine = [&](TreeNode* child){
        if (!child) return;
        sf::Vector2f s=node->position+sf::Vector2f(radius,radius);
        sf::Vector2f e=child->position+sf::Vector2f(radius,radius);
        sf::Vector2f d=e-s; float len=std::sqrt(d.x*d.x+d.y*d.y);
        if(len > 1.0f){ // Avoid division by zero
            d/=len;
            sf::Vector2f start = s + d * radius;
            sf::Vector2f end = e - d * radius;
            float lineLen = std::max(0.0f, len - radius * 2.f);
            sf::RectangleShape l(sf::Vector2f(lineLen, 2.f * std::max(0.6f, scaleFactor)));
            l.setPosition(start);
            l.setFillColor(sf::Color(150,150,150));
            l.setRotation(std::atan2(d.y,d.x)*180.f/3.14159f);
            window.draw(l);
        }
    };
    drawLine(node->left); drawLine(node->right);

    // Node color
    bool isPivot = (pivotValue != -1 && node->value == pivotValue);
    bool isUnbalanced = (unbalancedValue != -1 && node->value == unbalancedValue);
    bool hl = std::find(hlValues.begin(), hlValues.end(), node->value) != hlValues.end();

    sf::Color fill = sf::Color(0, 150, 80); // Default
    if (isUnbalanced) fill = sf::Color(255, 80, 80); // Red
    else if (isPivot) fill = sf::Color(80, 80, 255); // Blue
    else if (hl) fill = hlColor;
    
    sf::CircleShape circle(radius); circle.setPosition(node->position);
    circle.setFillColor(fill); circle.setOutlineThickness(2.f * std::max(0.6f, scaleFactor)); circle.setOutlineColor(sf::Color::White); window.draw(circle);

    sf::Text txt; txt.setFont(font); txt.setString(std::to_string(node->value));
    txt.setCharacterSize(static_cast<unsigned int>(std::max(10.f, 18 * scaleFactor))); txt.setFillColor(sf::Color::White);
    sf::FloatRect tb=txt.getLocalBounds(); txt.setOrigin(tb.left+tb.width/2.f,tb.top+tb.height/2.f);
    txt.setPosition(node->position+sf::Vector2f(radius,radius)); window.draw(txt);

    sf::Text bf; bf.setFont(font); bf.setString("bf:"+std::to_string(getBalance(node)));
    bf.setCharacterSize(static_cast<unsigned int>(std::max(8.f, 12 * scaleFactor))); bf.setFillColor(sf::Color(200,200,100));
    bf.setPosition(node->position.x+radius-10*std::max(0.6f, scaleFactor), node->position.y - radius - 5.f); window.draw(bf);

    drawNode(window, node->left,  hlValues, hlColor, pivotValue, unbalancedValue);
    drawNode(window, node->right, hlValues, hlColor, pivotValue, unbalancedValue);
}

// ---- draw ----
void AVLTree::draw(sf::RenderWindow& window) {
    float titleX = 200.f + 50.f; // Constant offset from sidebar
    sf::Text title; title.setFont(font); title.setString("AVL Tree");
    title.setCharacterSize(24); title.setFillColor(sf::Color::White); title.setPosition(titleX, 10); window.draw(title);

    std::vector<int> hlValues;
    sf::Color hlColor(220,180,0);
    int pVal = -1;
    int uVal = -1;
    if (animStep >= 0 && animStep < (int)animSteps.size()) {
        hlValues = animSteps[animStep].highlightedValues;
        hlColor  = animSteps[animStep].highlightColor;
        pVal     = animSteps[animStep].pivotValue;
        uVal     = animSteps[animStep].unbalancedValue;
        sf::Text msg; msg.setFont(font); 
        msg.setString(sf::String::fromUtf8(animSteps[animStep].message.begin(), animSteps[animStep].message.end()));
        msg.setCharacterSize(17); msg.setFillColor(sf::Color(200,230,255)); msg.setPosition(titleX, 40); window.draw(msg);
        
        sf::Text sc; sc.setFont(font); sc.setString("Step "+std::to_string(animStep+1)+"/"+std::to_string((int)animSteps.size()));
        sc.setCharacterSize(14); sc.setFillColor(sf::Color(160,160,160)); sc.setPosition(titleX, 60); window.draw(sc);
    }

    drawNode(window, root, hlValues, hlColor, pVal, uVal);
    for (auto& b : buttons) b.draw(window);
    for (auto& t : textInputs) t.draw(window);
}

void AVLTree::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
    for (auto& b : buttons) b.handleEvent(event, window);
    for (auto& t : textInputs) t.handleEvent(event, window);
}

void AVLTree::init(const std::vector<int>& data) {
    deleteTree(root); root = nullptr; 
    float rootX = 200.f + ((winW - 200.f - codePaneWidth) / 2.f);
    for (int i=0; i<(int)data.size(); ++i) root = insertNode(root, data[i], sf::Vector2f(rootX, 50.f));
}
void AVLTree::insert(int value) { 
    float rootX = 200.f + ((winW - 200.f - codePaneWidth) / 2.f);
    root = insertNode(root, value, sf::Vector2f(rootX, 50.f)); 
}
void AVLTree::remove(int value) { root = removeNode(root, value); }

void AVLTree::onResize(float w, float h) {
    winW = w; winH = h;
    codePaneWidth = w / 6.0f;
    initUI();
}
