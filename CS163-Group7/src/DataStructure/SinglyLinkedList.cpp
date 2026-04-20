#include "DataStructure/SinglyLinkedList.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
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

	float codeBoxW = 350.f;
	codeViewer.init(winW - codeBoxW, 0, codeBoxW, winH, &font);

	float btnW = 140.f;
	float btnH = 35.f;
	float gap = 10.f;
	int maxCols = std::max(1, (int)((winW - codeBoxW - 350.f) / (btnW + gap)));
	float startX = winW - codeBoxW + 30.f - (maxCols * btnW + (maxCols - 1) * gap);
	float colX = startX;
	float startY = winH - 150.f;
	int colCount = 0;

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Init Array", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Init Array", std::vector<InputDialog::Field>{{"Comma separated values:", "e.g. 10,20,30"}}, font, [this] (const std::vector<std::string> &results) {
			std::string t = results[0];
			std::vector<int> res;
			size_t pos = 0;
			while ((pos = t.find(',')) != std::string::npos) {
				if (pos > 0)
					res.push_back(std::stoi(t.substr(0, pos)));
				t.erase(0, pos + 1);
			}
			if (!t.empty()) {
				try {
					res.push_back(std::stoi(t));
				} catch (...) {
				}
			}
			if (!res.empty())
				init(res);
			activeDialog.reset();
		},
		[this] () { activeDialog.reset(); }, winW, winH, sf::Vector2f(colX + btnW + 15.f, startY));
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Init Random", font, [this] () {
		std::vector<int> r;
		for (int i = 0; i < 5; ++i)
			r.push_back(rand() % 100);
		init(r);
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Insert", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Insert Node", std::vector<InputDialog::Field>{{"Value:", "Val"}, {"Index (optional):", "Idx (default: end)"}}, font, [this] (const std::vector<std::string> &results) {
			if (!results[0].empty()) {
				try {
					int val = std::stoi(results[0]);
					int idx = results[1].empty() ? (int)nodes.size() : std::stoi(results[1]);
					insertSteps(val, idx);
				} catch (...) {
				}
			}
		activeDialog.reset();
		},
		[this] () { activeDialog.reset(); }, winW, winH, sf::Vector2f(colX + btnW + 15.f, startY));
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Insert Random", font, [this] () {
		insertSteps(rand() % 100, (int)nodes.size());
		}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Delete", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Delete Node", std::vector<InputDialog::Field>{{"Index to delete:", "Idx"}}, font, [this] (const std::vector<std::string> &results) {
			if (!results[0].empty()) {
				try {
					deleteSteps(std::stoi(results[0]));
				} catch (...) {
				}
			}
			activeDialog.reset();
		},
		[this] () { activeDialog.reset(); }, winW, winH,
		sf::Vector2f(colX + btnW + 15.f, startY));
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Update", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Update Node", std::vector<InputDialog::Field>{{"Index:", "Idx"}, {"New Value:", "Val"}}, font, [this] (const std::vector<std::string> &results) {
			if (!results[0].empty() && !results[1].empty()) {
				try {
					updateSteps(std::stoi(results[0]), std::stoi(results[1]));
				} catch (...) {
				}
			}
			activeDialog.reset();
		},
		[this] () { activeDialog.reset(); }, winW, winH,
		sf::Vector2f(colX + btnW + 15.f, startY));
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Search", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Search", std::vector<InputDialog::Field>{{"Value to search:", "Val"}}, font,[this] (const std::vector<std::string> &results) {
			if (!results[0].empty()) {
				try {
					searchSteps(std::stoi(results[0]));
				} catch (...) {
				}
			}
			activeDialog.reset();
		},
		[this] () { activeDialog.reset(); }, winW, winH,
		sf::Vector2f(colX + btnW + 15.f, startY));
	}));
}

void SinglyLinkedList::draw(sf::RenderWindow &window) {
	float centerX = (winW - 350.f) / 2.f;

	sf::Text title;
	title.setFont(font);
	title.setString("Singly Linked List");
	title.setCharacterSize(24);
	title.setFillColor(sf::Color::White);
	sf::FloatRect tb = title.getLocalBounds();
	title.setOrigin(tb.left + tb.width / 2.f, 0.f);
	title.setPosition(centerX, 10);
	window.draw(title);

	if (animStep >= 0 && animStep < (int)animSteps.size()) {
		sf::Text msg;
		msg.setFont(font);
		msg.setString(sf::String::fromUtf8(animSteps[animStep].message.begin(), animSteps[animStep].message.end()));
		msg.setCharacterSize(17);
		msg.setFillColor(sf::Color(200, 230, 255));
		sf::FloatRect mb = msg.getLocalBounds();
		msg.setOrigin(mb.left + mb.width / 2.f, 0.f);
		msg.setPosition(centerX, 40);
		window.draw(msg);

		sf::Text sc;
		sc.setFont(font);
		sc.setString("Step " + std::to_string(animStep + 1) + "/" + std::to_string((int)animSteps.size()));
		sc.setCharacterSize(14);
		sc.setFillColor(sf::Color(160, 160, 160));
		sf::FloatRect scb = sc.getLocalBounds();
		sc.setOrigin(scb.left + scb.width / 2.f, 0.f);
		sc.setPosition(centerX, 60);
		window.draw(sc);

		if (!animSteps[animStep].highlightedCodeLines.empty())
			codeViewer.setActiveLines(animSteps[animStep].highlightedCodeLines);
		else
			codeViewer.setActiveLines({});
	}

	float radius = 25.f;

	for (size_t i = 0; i < nodes.size(); ++i) {
		if (i < nodes.size() - 1) {
			sf::Vector2f s = nodes[i].position + sf::Vector2f(radius, radius);
			sf::Vector2f e = nodes[i + 1].position + sf::Vector2f(radius, radius);
			sf::Vector2f d = e - s;
			float len = std::sqrt(d.x * d.x + d.y * d.y);

			if (len > radius * 2) {
				d /= len;
				s += d * radius;
				e -= d * radius;
				len -= radius * 2.f;

				sf::RectangleShape line(sf::Vector2f(len, 2.f));
				line.setPosition(s);
				line.setFillColor(sf::Color::White);

				float angle = std::atan2(d.y, d.x) * 180.f / 3.14159f;
				line.setRotation(angle);
				window.draw(line);

				sf::ConvexShape arr(3);
				arr.setPoint(0, {0, 0});
				arr.setPoint(1, {-10, 5});
				arr.setPoint(2, {-10, -5});
				arr.setFillColor(sf::Color::White);
				arr.setPosition(e);
				arr.setRotation(angle);
				window.draw(arr);
			}
		}

		sf::Color fillColor(0, 120, 215);

		if (animStep >= 0 && animStep < (int)animSteps.size()) {
			for (int hi : animSteps[animStep].highlighted) {
				if (hi == (int)i) {
					fillColor = animSteps[animStep].highlightColor;
					break;
				}
			}
		}

		sf::CircleShape circle(radius);
		circle.setPosition(nodes[i].position);
		circle.setFillColor(fillColor);
		circle.setOutlineThickness(2.f);
		circle.setOutlineColor(sf::Color::White);
		window.draw(circle);

		sf::Text txt;
		txt.setFont(font);
		txt.setString(std::to_string(nodes[i].value));
		txt.setCharacterSize(20);
		txt.setFillColor(sf::Color::White);

		sf::FloatRect tb = txt.getLocalBounds();
		txt.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
		txt.setPosition(nodes[i].position + sf::Vector2f(radius, radius));
		window.draw(txt);

		sf::Text idx;
		idx.setFont(font);
		idx.setString(std::to_string(i));
		idx.setCharacterSize(14);
		idx.setFillColor(sf::Color(200, 200, 200));
		idx.setPosition(nodes[i].position.x + radius - 5, nodes[i].position.y + radius * 2.f + 5);
		window.draw(idx);
	}
	for (auto &b : buttons)
		b.draw(window);
	codeViewer.draw(window);

	if (activeDialog) {
		activeDialog->draw(window);
	}
}

void SinglyLinkedList::handleEvent(const sf::Event &event, const sf::RenderWindow &window) {
	if (activeDialog) {
		activeDialog->handleEvent(event, window);
		return;
	}

	for (auto &b : buttons)
		b.handleEvent(event, window);
}

void SinglyLinkedList::onResize(float w, float h) {
	winW = w;
	winH = h;

	initUI();

	if (!nodes.empty()) {
		float codeBoxW = 350.f;
		float availW = winW - 250.f - codeBoxW;
		float totalWidth = nodes.size() * 100.f;
		float startX = 250.f + ((availW - totalWidth) / 2.f);

		for (size_t i = 0; i < nodes.size(); ++i) {
			nodes[i].targetPosition =sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
			nodes[i].position = nodes[i].targetPosition;
		}
	}
}

void SinglyLinkedList::init(const std::vector<int> &data) {
	nodes.clear();
	animSteps.clear();
	animStep = -1;
	codeViewer.setCode({});

	float codeBoxW = 350.f;
	float availW = winW - 250.f - codeBoxW;

	float totalWidth = data.size() * 100.f;
	float startX = 250.f + ((availW - totalWidth) / 2.f);

	for (size_t i = 0; i < data.size(); ++i) {
		Node n;
		n.value = data[i];
		n.targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
		n.position = sf::Vector2f(startX + i * 100.f, -50.f);
		nodes.push_back(n);
	}
}

void SinglyLinkedList::insert(int value, int index) {
	if (index < 0)
		index = 0;

	if (index > (int)nodes.size())
		index = (int)nodes.size();

	Node n;
	n.value = value;

	float codeBoxW = 350.f;
	float availW = winW - codeBoxW;

	n.position = sf::Vector2f(availW / 2.f, winH + 100.f);
	nodes.insert(nodes.begin() + index, n);

	float totalWidth = nodes.size() * 100.f;
	float startX = ((availW - totalWidth) / 2.f);

	for (size_t i = 0; i < nodes.size(); ++i)
		nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
}

void SinglyLinkedList::remove(int index) {
	if (index < 0 || index >= (int)nodes.size())
		return;

	nodes.erase(nodes.begin() + index);

	float codeBoxW = 350.f;
	float availW = winW - codeBoxW;

	float totalWidth = nodes.size() * 100.f;
	float startX = ((availW - totalWidth) / 2.f);

	for (size_t i = 0; i < nodes.size(); ++i)
		nodes[i].targetPosition = sf::Vector2f(startX + i * 100.f, winH / 2.f - 50.f);
}

void SinglyLinkedList::update(float dt) {
	for (auto &n : nodes) {
		if (std::isnan(n.position.x) || std::isnan(n.position.y))
			n.position = sf::Vector2f(350.f, -50.f);

		if (std::isnan(n.targetPosition.x) || std::isnan(n.targetPosition.y))
			n.targetPosition = sf::Vector2f(350.f, 300.f);

		n.position.x += (n.targetPosition.x - n.position.x) * 12.f * dt;
		n.position.y += (n.targetPosition.y - n.position.y) * 12.f * dt;
	}

	if (isPlaying && animStep >= 0) {
		playTimer += dt;
		if (playTimer >= playInterval) {
			playTimer = 0.f;
			stepForward();
		}
	}
}

void SinglyLinkedList::play() {
	isPlaying = true;
	playTimer = 0.f;
}

void SinglyLinkedList::pause() { isPlaying = false; }

void SinglyLinkedList::stepForward() {
	if (animStep < 0 || animSteps.empty())
		return;

	if (animStep + 1 < (int)animSteps.size()) {
		animStep++;

		if (animStep == (int)animSteps.size() - 1) {
			if (commitOp) {
				commitOp();
				commitOp = nullptr;
			}

			isPlaying = false;
		}
	} else
		isPlaying = false;
}

void SinglyLinkedList::stepBackward() {
	if (animStep > 0)
		animStep--;
}

void SinglyLinkedList::prepareNewOperation() {
	if (commitOp) {
		commitOp();
		commitOp = nullptr;
	}
	animSteps.clear();
	commitOp = nullptr;
	isPlaying = false;
	playTimer = 0.f;
}

void SinglyLinkedList::finalizeOperation(std::function<void()> finalCommit) {
	commitOp = finalCommit;
	animStep = 0;
	isPlaying = true;
}

void SinglyLinkedList::insertSteps(int value, int idx) {
	prepareNewOperation();

	if (idx < 0)
		idx = 0;

	if (idx > (int)nodes.size())
		idx = (int)nodes.size();

	if (!isStepByStep) {
		insert(value, idx);

		VisualStep s;
		s.message = "Inserted " + std::to_string(value) + " at index " +
			std::to_string(idx) + " (Run at Once)";
		s.highlightColor = sf::Color(0, 200, 80);

		if (idx < (int)nodes.size())
			s.highlighted = {idx};

		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {"void insert(int index, int value) {",
									"    Node* current = head;",
									"    for (int i=0; i < index - 1; i++)",
									"        current = current->next;",
									"    Node* newNode = new Node(value);",
									"    newNode->next = current->next;",
									"    current->next = newNode;",
									"}"};
	codeViewer.setCode(code);

	for (int i = 0; i < idx && i < (int)nodes.size(); ++i) {
		VisualStep s;
		s.highlighted = {i};

		std::vector<int> lines;
		if (i == 0)
			lines = {1};
		else
			lines = {2, 3};

		s.highlightedCodeLines = lines;
		s.message = "Traversing to index " + std::to_string(idx) + "→ at node[" +
			std::to_string(i) + "] = " + std::to_string(nodes[i].value);
		animSteps.push_back(s);
	}

	VisualStep fin;
	fin.highlighted = {idx};
	fin.highlightedCodeLines = {4, 5, 6};
	fin.message = "Will insert " + std::to_string(value) + " at index " +
		std::to_string(idx);
	fin.highlightColor = sf::Color(0, 200, 80);
	animSteps.push_back(fin);

	VisualStep done;
	done.highlighted = {idx};
	done.highlightedCodeLines = {};
	done.message = "Inserted " + std::to_string(value) + " at index " +
		std::to_string(idx) + "!";
	done.highlightColor = sf::Color(0, 200, 80);
	animSteps.push_back(done);

	finalizeOperation([this, value, idx] () { insert(value, idx); });
}

void SinglyLinkedList::deleteSteps(int idx) {
	prepareNewOperation();

	if (idx < 0 || idx >= (int)nodes.size())
		return;

	int value = nodes[idx].value;

	if (!isStepByStep) {
		remove(idx);

		VisualStep s;
		s.message = "Deleted node at index " + std::to_string(idx) +
			" with value " + std::to_string(value) + " (Run at Once)";
		s.highlightColor = sf::Color(220, 60, 60);

		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {"void remove(int index) {",
									"    Node* current = head;",
									"    for (int i=0; i < index - 1; i++)",
									"        current = current->next;",
									"    Node* temp = current->next;",
									"    current->next = temp->next;",
									"    delete temp;",
									"}"};
	codeViewer.setCode(code);

	for (int i = 0; i <= idx && i < (int)nodes.size(); ++i) {
		VisualStep s;
		s.highlighted = {i};
		s.highlightedCodeLines = (i == 0) ? std::vector<int>{1} : std::vector<int> {2, 3};

		if (i < idx) {
			s.message = "Searching for index " + std::to_string(idx) + " — at node[" +	std::to_string(i) + "] = " + std::to_string(nodes[i].value);
		} else {
			s.message = "Found node[" + std::to_string(idx) + "] = " + std::to_string(value) + ". Removing...";
			s.highlightColor = sf::Color(220, 60, 60);
			s.highlightedCodeLines = {4, 5, 6};
		}

		animSteps.push_back(s);
	}

	VisualStep done;
	done.message = "Deleted node at index " + std::to_string(idx) + "!";
	done.highlightColor = sf::Color(220, 60, 60);
	done.highlightedCodeLines = {};
	animSteps.push_back(done);

	finalizeOperation([this, idx] () { remove(idx); });
}

void SinglyLinkedList::searchSteps(int value) {
	prepareNewOperation();

	bool found = false;
	int foundIdx = -1;

	if (!isStepByStep) {
		for (int i = 0; i < (int)nodes.size(); ++i) {
			if (nodes[i].value == value) {
				found = true;
				foundIdx = i;
				break;
			}
		}

		VisualStep s;

		if (found) {
			s.message = "Found " + std::to_string(value) + " at index " +
				std::to_string(foundIdx) + "! (Run at Once)";
			s.highlightColor = sf::Color(0, 200, 80);
			s.highlighted = {foundIdx};
		} else {
			s.message =
				std::to_string(value) + " not found in the list. (Run at Once)";
			s.highlightColor = sf::Color(220, 60, 60);
		}

		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {"bool search(int value) {",
									"    Node* current = head;",
									"    while (current != nullptr) {",
									"        if (current->value == value)",
									"            return true;",
									"        current = current->next;",
									"    }",
									"    return false;",
									"}"};
	codeViewer.setCode(code);

	for (int i = 0; i < (int)nodes.size(); ++i) {
		VisualStep s;
		s.highlighted = {i};

		std::vector<int> curLines = (i == 0) ? std::vector<int>{1, 2, 3} : std::vector<int> {2, 3, 5};

		if (nodes[i].value == value) {
			s.highlightedCodeLines = {2, 3, 4};
			s.message = "Found " + std::to_string(value) + " at index " + std::to_string(i) + "!";
			s.highlightColor = sf::Color(0, 200, 80);
			animSteps.push_back(s);
			found = true;
			foundIdx = i;
			break;
		} else {
			s.highlightedCodeLines = curLines;
			s.message = "Searching for " + std::to_string(value) + " — current node is " + std::to_string(nodes[i].value);
			animSteps.push_back(s);
		}
	}

	VisualStep done;

	if (found) {
		done.message = "Search finished! Found at index " + std::to_string(foundIdx);
		done.highlightColor = sf::Color(0, 200, 80);
		done.highlighted = {foundIdx};
		done.highlightedCodeLines = {};
	} else {
		VisualStep s;
		s.message = std::to_string(value) + " not found in the list.";
		s.highlightColor = sf::Color(220, 60, 60);
		s.highlightedCodeLines = {2, 7};
		animSteps.push_back(s);

		done.message = "Search finished! Not found.";
		done.highlightColor = sf::Color(220, 60, 60);
		done.highlightedCodeLines = {};
	}

	animSteps.push_back(done);

	finalizeOperation(nullptr);
}

void SinglyLinkedList::updateSteps(int idx, int newValue) {
	if (idx < 0 || idx >= (int)nodes.size())
		return;

	prepareNewOperation();

	if (!isStepByStep) {
		nodes[idx].value = newValue;
		VisualStep s;
		s.message = "Updated node at index " + std::to_string(idx) + " to " + std::to_string(newValue) + " (Run at Once)";
		s.highlightColor = sf::Color(100, 180, 255);
		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {"void updateNode(int idx, int newValue) {",
									 "    Node* curr = head;",
									 "    for (int i = 0; i < idx; ++i)",
									 "        curr = curr->next;",
									 "    curr->value = newValue;",
									 "}"};
	codeViewer.setCode(code);

	int curr = 0;

	{
		VisualStep s;
		s.highlighted = {0};
		s.message = "Start at head, curr = 0";
		s.highlightedCodeLines = {1};
		s.highlightColor = sf::Color(100, 180, 255);
		animSteps.push_back(s);
	}

	while (curr < idx) {
		{
			VisualStep s;
			s.highlighted = {curr};
			s.message = "curr != target, moving... (" + std::to_string(curr) + " < " + std::to_string(idx) + ")";
			s.highlightedCodeLines = {2};
			s.highlightColor = sf::Color(100, 180, 255);
			animSteps.push_back(s);
		}
		curr++;
		{
			VisualStep s;
			s.highlighted = {curr};
			s.message = "curr = curr->next";
			s.highlightedCodeLines = {3};
			s.highlightColor = sf::Color(100, 180, 255);
			animSteps.push_back(s);
		}
	}

	{
		VisualStep s;
		s.highlighted = {idx};
		s.message = "Found node at index " + std::to_string(idx) + "!";
		s.highlightedCodeLines = {2};
		s.highlightColor = sf::Color(0, 200, 80);
		animSteps.push_back(s);
	}

	{
		VisualStep s;
		s.highlighted = {idx};
		s.message = "Change value from " + std::to_string(nodes[idx].value) + " to " + std::to_string(newValue);
		s.highlightedCodeLines = {4};
		s.highlightColor = sf::Color(100, 180, 255);
		animSteps.push_back(s);
	}

	{
		VisualStep done;
		done.message = "Update completed!";
		done.highlightColor = sf::Color(100, 180, 255);
		animSteps.push_back(done);
	}

	finalizeOperation([this, idx, newValue] () {
		if (idx < (int)nodes.size())
			nodes[idx].value = newValue;
	});
}
