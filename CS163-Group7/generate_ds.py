import os

names = [
    "DoublyLinkedList",
    "CircularLinkedList",
    "MinMaxHeap",
    "HashChaining",
    "HashLinearProbing",
    "HashQuadraticProbing",
    "HashDoubleHashing",
    "AVLTree",
    "TwoThreeTree",
    "TwoThreeFourTree",
    "RedBlackTree",
    "GraphVisualizer"
]

header_template = """#pragma once
#include "DataStructure/DataStructure.hpp"
#include <SFML/Graphics.hpp>
#include <vector>
#include "UI/Button.hpp"
#include "UI/TextInput.hpp"

class {name} : public DataStructure {{
public:
    {name}();
    
    void update(float dt) override;
    void draw(sf::RenderWindow& window) override;
    void handleEvent(const sf::Event& event, const sf::RenderWindow& window) override;
    
    void play() override;
    void pause() override;
    void stepForward() override;
    void stepBackward() override;

private:
    sf::Font font;
    sf::Text titleText;
    std::vector<Button> buttons;
    std::vector<TextInput> textInputs;
    
    void initUI();
}};
"""

cpp_template = """#include "DataStructure/{name}.hpp"
#include <iostream>

{name}::{name}() {{
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {{
        std::cerr << "Error loading font for {name}\\n";
    }}
    titleText.setFont(font);
    titleText.setString("{name} - Not Implemented Yet");
    titleText.setCharacterSize(24);
    titleText.setFillColor(sf::Color::White);
    titleText.setPosition(400, 300);
    initUI();
}}

void {name}::initUI() {{
    // Add initialization UI here later
}}

void {name}::update(float dt) {{}}

void {name}::draw(sf::RenderWindow& window) {{
    window.draw(titleText);
    for (auto& btn : buttons) btn.draw(window);
    for (auto& input : textInputs) input.draw(window);
}}

void {name}::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {{
    for (auto& btn : buttons) btn.handleEvent(event, window);
    for (auto& input : textInputs) input.handleEvent(event, window);
}}

void {name}::play() {{}}
void {name}::pause() {{}}
void {name}::stepForward() {{}}
void {name}::stepBackward() {{}}
"""

inc_dir = "include/DataStructure"
src_dir = "src/DataStructure"

os.makedirs(inc_dir, exist_ok=True)
os.makedirs(src_dir, exist_ok=True)

for name in names:
    with open(f"{inc_dir}/{name}.hpp", "w") as f:
        f.write(header_template.format(name=name))
    with open(f"{src_dir}/{name}.cpp", "w") as f:
        f.write(cpp_template.format(name=name))

print("Successfully generated all DS boilerplate.")
