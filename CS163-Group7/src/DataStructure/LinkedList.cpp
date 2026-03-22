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
}

LinkedList::~LinkedList() {}
