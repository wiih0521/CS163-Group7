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
    delete n;
}

void AVLTree::clearAnimSteps() {
    for (auto& s : animSteps) { if (s.treeSnapshot) deleteSimTree(s.treeSnapshot); }
    animSteps.clear();
}

void AVLTree::initUI() {
    buttons.clear();
    textInputs.clear();
    
    float uiY1 = winH - 160.f;
    float uiY2 = winH - 120.f;
    
    textInputs.push_back(TextInput(sf::Vector2f(120,30), sf::Vector2f(300, uiY1), font, "e.g. 10,20,30"));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(300, uiY2), "Init Array", font, [this]() {
        std::string t = textInputs[0].getText(); std::vector<int> res; size_t pos=0;
        while ((pos=t.find(','))!=std::string::npos){if(pos>0)res.push_back(std::stoi(t.substr(0,pos)));t.erase(0,pos+1);}
        if (!t.empty()) res.push_back(std::stoi(t)); if (!res.empty()) init(res);
    }));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(430, uiY2), "Init Random", font, [this]() {
        std::vector<int> r; for (int i=0;i<10;++i) r.push_back(rand()%100); init(r);
    }));
    textInputs.push_back(TextInput(sf::Vector2f(60,30), sf::Vector2f(560, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(560, uiY2), "Insert", font, [this]() {
        std::string v = textInputs[1].getText(); if (!v.empty()) beginInsertSteps(std::stoi(v));
    }));
    buttons.push_back(Button(sf::Vector2f(110,30), sf::Vector2f(690, uiY2), "Add Random", font, [this]() {
        beginInsertSteps(rand()%100);
    }));
    textInputs.push_back(TextInput(sf::Vector2f(60,30), sf::Vector2f(810, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(120,30), sf::Vector2f(810, uiY2), "Delete", font, [this]() {
        std::string v = textInputs[2].getText(); if (!v.empty()) beginDeleteSteps(std::stoi(v));
    }));
    textInputs.push_back(TextInput(sf::Vector2f(60,30), sf::Vector2f(940, uiY1), font, "Value"));
    buttons.push_back(Button(sf::Vector2f(90,30), sf::Vector2f(940, uiY2), "Search", font, [this]() {
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
    auto takeSnapshot = [&](const std::string& msg, const std::vector<int>& hl, sf::Color clr, int pivot = -1, int unbal = -1) {
        VisualStep s; s.message = msg; s.highlightedValues = hl; s.highlightColor = clr;
        s.pivotValue = pivot; s.unbalancedValue = unbal;
        s.treeSnapshot = copySimTree(*rootRef);
        steps.push_back(s);
    };

    SimNode* node = *nodeRef;
    if (!node) {
        *nodeRef = new SimNode{value, 1, nullptr, nullptr};
        takeSnapshot("Leaf reached: Inserting " + std::to_string(value) + " here.", {value}, sf::Color(0, 200, 80));
        return;
    }

    takeSnapshot("Insert " + std::to_string(value) + ": looking at " + std::to_string(node->value), {node->value}, sf::Color(220, 180, 0));

    if (value < node->value) {
        insertNodeSim(&(node->left), value, steps, rootRef);
    } else if (value > node->value) {
        insertNodeSim(&(node->right), value, steps, rootRef);
    } else {
        takeSnapshot(std::to_string(value) + " already exists!", {node->value}, sf::Color(255, 60, 60));
        return;
    }

    
    node = *nodeRef;
    node->height = 1 + std::max(getSimHeight(node->left), getSimHeight(node->right));
    int bal = getSimBalance(node);

    takeSnapshot("Checking balance of " + std::to_string(node->value) + " (bf=" + std::to_string(bal) + ")", {node->value}, sf::Color(220, 180, 0));

    
    if (bal > 1 && value < (node->left ? node->left->value : -1e9)) {
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("Performing Right Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
        return;
    }
    
    if (bal < -1 && value > (node->right ? node->right->value : 1e9)) {
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("Performing Left Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
        return;
    }
    
    if (bal > 1 && value > (node->left ? node->left->value : 1e9)) {
        int childVal = node->left->value;
        node->left = rotateLeftSim(node->left);
        takeSnapshot("Left-Right Case: First rotating left on child " + std::to_string(childVal), {node->left->value}, sf::Color(255, 150, 100));
        
        node = *nodeRef; 
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("Left-Right Case: Now rotating right on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
        return;
    }
    
    if (bal < -1 && value < (node->right ? node->right->value : -1e9)) {
        int childVal = node->right->value;
        node->right = rotateRightSim(node->right);
        takeSnapshot("Right-Left Case: First rotating right on child " + std::to_string(childVal), {node->right->value}, sf::Color(255, 150, 100));
        
        node = *nodeRef; 
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("Right-Left Case: Now rotating left on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
        return;
    }
}

void AVLTree::removeNodeSim(SimNode** nodeRef, int value, std::vector<VisualStep>& steps, SimNode** rootRef) {
    auto takeSnapshot = [&](const std::string& msg, const std::vector<int>& hl, sf::Color clr, int pivot = -1, int unbal = -1) {
        VisualStep s; s.message = msg; s.highlightedValues = hl; s.highlightColor = clr;
        s.pivotValue = pivot; s.unbalancedValue = unbal;
        s.treeSnapshot = copySimTree(*rootRef);
        steps.push_back(s);
    };

    SimNode* node = *nodeRef;
    if (!node) return;

    takeSnapshot("Delete " + std::to_string(value) + ": looking at " + std::to_string(node->value), {node->value}, sf::Color(220, 180, 0));

    if (value < node->value) {
        removeNodeSim(&(node->left), value, steps, rootRef);
    } else if (value > node->value) {
        removeNodeSim(&(node->right), value, steps, rootRef);
    } else {
        VisualStep s; s.message = "Found " + std::to_string(value) + "! Removing node...";
        s.highlightColor = sf::Color(255, 60, 60);
        s.highlightedValues = {node->value};
        s.treeSnapshot = copySimTree(*rootRef);
        steps.push_back(s);

        if (!node->left || !node->right) {
            SimNode* t = node->left ? node->left : node->right;
            if (!t) { *nodeRef = nullptr; delete node; return; }
            else { 
                SimNode* res = new SimNode{*t}; 
                *nodeRef = res; delete node; 
                takeSnapshot("Replacing node with its child", {res->value}, sf::Color(200, 100, 255));
                return; 
            }
        } else {
            SimNode* successor = minValueNodeSim(node->right);
            int succVal = successor->value;
            node->value = succVal;
            removeNodeSim(&(node->right), succVal, steps, rootRef);
            
            node = *nodeRef;
        }
    }

    node = *nodeRef;
    if (!node) return;
    node->height = 1 + std::max(getSimHeight(node->left), getSimHeight(node->right));
    int bal = getSimBalance(node);

    takeSnapshot("Checking balance of " + std::to_string(node->value) + " (bf=" + std::to_string(bal) + ")", {node->value}, sf::Color(220, 180, 0));

    if (bal > 1 && getSimBalance(node->left) >= 0) {
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("Performing Right Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
    } else if (bal > 1 && getSimBalance(node->left) < 0) {
        int childVal = node->left->value;
        node->left = rotateLeftSim(node->left);
        takeSnapshot("Double Rotation: Rotating left on child " + std::to_string(childVal), {node->left->value}, sf::Color(255, 150, 100));
        
        node = *nodeRef;
        int oldVal = node->value; int pivotVal = node->left->value;
        *nodeRef = rotateRightSim(*nodeRef);
        takeSnapshot("Double Rotation: Now rotating right on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
    } else if (bal < -1 && getSimBalance(node->right) <= 0) {
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("Performing Left Rotation on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
    } else if (bal < -1 && getSimBalance(node->right) > 0) {
        int childVal = node->right->value;
        node->right = rotateRightSim(node->right);
        takeSnapshot("Double Rotation: Rotating right on child " + std::to_string(childVal), {node->right->value}, sf::Color(255, 150, 100));
        
        node = *nodeRef;
        int oldVal = node->value; int pivotVal = node->right->value;
        *nodeRef = rotateLeftSim(*nodeRef);
        takeSnapshot("Double Rotation: Now rotating left on " + std::to_string(oldVal), {oldVal, pivotVal}, sf::Color(255, 100, 100), pivotVal, oldVal);
    }
}

AVLTree::SimNode* AVLTree::minValueNodeSim(SimNode* node) {
    SimNode* cur = node; while (cur->left) cur = cur->left; return cur;
}


void AVLTree::applySimStructure(SimNode* simRoot) {
    std::set<TreeNode*> usedNodes;
    auto sync = [&](auto self, SimNode* sn) -> TreeNode* {
        if (!sn) return nullptr;
        TreeNode* tn = nullptr;
        if (nodeCache.count(sn->value)) {
            tn = nodeCache[sn->value];
            
            
            if (usedNodes.count(tn)) {
                tn = new TreeNode(sn->value);
                
                tn->position = tn->targetPosition = nodeCache[sn->value]->position;
            } else {
                usedNodes.insert(tn);
            }
        } else {
            tn = new TreeNode(sn->value);
            float rootX = 250.f + ((winW - 250.f) / 2.f);
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


void AVLTree::beginInsertSteps(int value) {
    clearAnimSteps(); commitOp = nullptr; isPlaying = false; playTimer = 0.f; nodeCache.clear();
    
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
        VisualStep s; s.highlightedValues = {curr->value};
        if (value < curr->value) {
            s.message = "Search "+std::to_string(value)+": "+std::to_string(value)+" < "+std::to_string(curr->value)+" -> go LEFT";
            animSteps.push_back(s); curr = curr->left;
        } else if (value > curr->value) {
            s.message = "Search "+std::to_string(value)+": "+std::to_string(value)+" > "+std::to_string(curr->value)+" -> go RIGHT";
            animSteps.push_back(s); curr = curr->right;
        } else {
            s.message = "Found "+std::to_string(value)+"!";
            s.highlightColor = sf::Color(0,200,80); 
            animSteps.push_back(s); found = true; break;
        }
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


void AVLTree::update(float dt) {
    float rootX = 250.f + ((winW - 250.f) / 2.f);
    
    
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

    float availW = (winW - 250.f) * 0.95f;
    float availH = (winH - 150.f) * 0.95f;

    
    float scaleW = (requiredWidth > 0) ? availW / requiredWidth : 1.0f;
    float scaleH = (requiredHeight > 0) ? availH / requiredHeight : 1.0f;
    scaleFactor = std::min({1.0f, scaleW, scaleH});
    
    
    if (scaleFactor < 0.2f) scaleFactor = 0.2f;

    float hs = baseNodeGap * scaleFactor;
    float vs = baseVerticalGap * scaleFactor;
    
    
    float minRadius = 14.f;
    if (hs < minRadius * 2.2f) hs = minRadius * 2.2f;

    float totalTreeWidth = totalNodes * hs;
    float startX = 250.f + ((winW - 250.f - totalTreeWidth) / 2.f) + (hs / 2.f);

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


void AVLTree::drawNode(sf::RenderWindow& window, TreeNode* node,
                       const std::vector<int>& hlValues, sf::Color hlColor,
                       int pivotValue, int unbalancedValue) {
    if (!node) return;
    float radius = std::max(14.f, 22.f * scaleFactor);

    
    auto drawLine = [&](TreeNode* child){
        if (!child) return;
        sf::Vector2f s=node->position+sf::Vector2f(radius,radius);
        sf::Vector2f e=child->position+sf::Vector2f(radius,radius);
        sf::Vector2f d=e-s; float len=std::sqrt(d.x*d.x+d.y*d.y);
        if(len > 1.0f){ 
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

    
    bool isPivot = (pivotValue != -1 && node->value == pivotValue);
    bool isUnbalanced = (unbalancedValue != -1 && node->value == unbalancedValue);
    bool hl = std::find(hlValues.begin(), hlValues.end(), node->value) != hlValues.end();

    sf::Color fill = sf::Color(0, 150, 80); 
    if (isUnbalanced) fill = sf::Color(255, 80, 80); 
    else if (isPivot) fill = sf::Color(80, 80, 255); 
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


void AVLTree::draw(sf::RenderWindow& window) {
    sf::Text title; title.setFont(font); title.setString("AVL Tree");
    title.setCharacterSize(24); title.setFillColor(sf::Color::White); title.setPosition(300,10); window.draw(title);

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
        msg.setCharacterSize(17); msg.setFillColor(sf::Color(200,230,255)); msg.setPosition(300,40); window.draw(msg);
        
        sf::Text sc; sc.setFont(font); sc.setString("Step "+std::to_string(animStep+1)+"/"+std::to_string((int)animSteps.size()));
        sc.setCharacterSize(14); sc.setFillColor(sf::Color(160,160,160)); sc.setPosition(300,60); window.draw(sc);
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
    float rootX = 250.f + ((winW - 250.f) / 2.f);
    for (int i=0; i<(int)data.size(); ++i) root = insertNode(root, data[i], sf::Vector2f(rootX, 50.f));
}
void AVLTree::insert(int value) { 
    float rootX = 250.f + ((winW - 250.f) / 2.f);
    root = insertNode(root, value, sf::Vector2f(rootX, 50.f)); 
}
void AVLTree::remove(int value) { root = removeNode(root, value); }

void AVLTree::onResize(float w, float h) {
    winW = w; winH = h;
    initUI();
}