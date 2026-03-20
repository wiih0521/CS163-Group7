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
AVLTree::~AVLTree() { deleteTree(root); }

void AVLTree::deleteTree(TreeNode* n) {
    if (!n) return; deleteTree(n->left); deleteTree(n->right); 
    // Do NOT delete if it might be in nodeCache? Actually deleteTree is for real root.
    // We should probably clear nodeCache too.
    delete n;
}

// ---- Step helpers ----
void AVLTree::beginInsertSteps(int value) {
    clearAnimSteps(); commitOp = nullptr; isPlaying = false; playTimer = 0.f; nodeCache.clear();
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
            s.highlightColor = sf::Color(0,200,80); // Green
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
