#include "DataStructure/MinMaxHeap.h"
#include "UI/Theme.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

MinMaxHeap::MinMaxHeap(float windowWidth, float windowHeight) : isMinHeap(true) {
	winW = windowWidth;
	winH = windowHeight;
	if (!font.loadFromFile("assets/fonts/arial.ttf"))
		std::cerr << "Failed to load font\n";
	initUI();
}

MinMaxHeap::~MinMaxHeap() {}

void MinMaxHeap::initUI() {
	buttons.clear();

	float codeBoxW = 350.f;
	codeViewer.init(winW - codeBoxW, 0, codeBoxW, winH, &font);

	float btnW = 230.f;
	float btnH = 45.f;
	float gap = 10.f;
	int maxCols = std::max(1, (int)((winW - codeBoxW - 480.f) / (btnW + gap)));
	float startX = winW - codeBoxW - (maxCols * btnW + (maxCols - 1) * gap) - 15.f;
	float colX = startX;
	float startY = winH - 150.f;
	int colCount = 0;
	
	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Init Array", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Init Array", std::vector<InputDialog::Field>{{"Comma separated values:", "e.g. 10,20,30"}}, font, [this] (const std::vector<std::string>& results) {
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
		for (int i = 0; i < 15; ++i)
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

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Push", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Push Value", std::vector<InputDialog::Field>{{"Value:", "Val"}}, font, [this] (const std::vector<std::string>& results) {
			if (!results[0].empty()) {
				try {
					insertSteps(std::stoi(results[0]));
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

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Push Random", font, [this] () {
		insertSteps(rand() % 100);
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Pop", font, [this] () {
		extractSteps();
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
		activeDialog = std::make_unique<InputDialog>("Search", std::vector<InputDialog::Field>{{"Value to search:", "Val"}}, font, [this] (const std::vector<std::string>& results) {
			if (!results[0].empty()) {
				try {
					searchSteps(std::stoi(results[0]));
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

	buttons.push_back(Button(sf::Vector2f(btnW + 30.f, btnH), sf::Vector2f(colX, startY), "Min / Max Toggle", font, [this] () {
		isMinHeap = !isMinHeap;
		if (!rawData.empty()) buildHeap();
	}));
}

void MinMaxHeap::draw(sf::RenderWindow& window) {
	float radius = 22.f;
	float centerX = (winW - 350.f) / 2.f;

	if (animStep >= 0 && animStep < (int)animSteps.size()) {
		sf::Text msg;
		msg.setFont(font);
		msg.setString(sf::String::fromUtf8(animSteps[animStep].message.begin(), animSteps[animStep].message.end()));
		msg.setCharacterSize(17);
		msg.setFillColor(sf::Color(100, 170, 230));
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

	for (size_t i = 0; i < nodes.size(); ++i) {
		auto drawLine = [&] (size_t child) {
			if (child >= nodes.size()) return;
			sf::Vector2f s = nodes[i].position + sf::Vector2f(radius, radius);
			sf::Vector2f e = nodes[child].position + sf::Vector2f(radius, radius);
			sf::Vector2f d = e - s;
			float len = std::sqrt(d.x * d.x + d.y * d.y);
			if (len > radius * 2) {
				d /= len;
				s += d * radius;
				e -= d * radius;
				len -= radius * 2.f;
				sf::RectangleShape line(sf::Vector2f(len, 2.f));
				line.setPosition(s);
				line.setFillColor(sf::Color(150, 150, 150));
				line.setRotation(std::atan2(d.y, d.x) * 180.f / 3.14159f);
				window.draw(line);
			}
		};
		drawLine(2 * i + 1);
		drawLine(2 * i + 2);
	}

	for (size_t i = 0; i < nodes.size(); ++i) {
		sf::Color fillColor = isMinHeap ? sf::Color(0, 150, 100) : sf::Color(150, 0, 100);
		if (animStep >= 0 && animStep < (int)animSteps.size()) {
			const auto& step = animSteps[animStep];
			if (!step.highlighted.empty() && step.highlighted[0] == (int)i)
				fillColor = step.highlightColor;
			else if (step.highlighted.size() > 1 && step.highlighted[1] == (int)i)
				fillColor = step.highlightColor2;
		}

		sf::CircleShape circle(radius);
		circle.setPosition(nodes[i].position);
		circle.setFillColor(fillColor);
		circle.setOutlineThickness(2.f);
		// circle.setOutlineColor(sf::Color::White);
		circle.setOutlineColor(sf::Color(90, 90, 90));
		window.draw(circle);

		sf::Text txt;
		txt.setFont(font);
		txt.setString(std::to_string(nodes[i].value));
		txt.setCharacterSize(18);
		txt.setFillColor(sf::Color::White);
		sf::FloatRect tb = txt.getLocalBounds();
		txt.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
		txt.setPosition(nodes[i].position + sf::Vector2f(radius, radius));
		window.draw(txt);

		sf::Text id;
		id.setFont(font);
		id.setString("[" + std::to_string(i) + "]");
		id.setCharacterSize(12);
		id.setFillColor(sf::Color(200, 200, 200));
		id.setPosition(nodes[i].position.x + radius - 10, nodes[i].position.y - 20);
		window.draw(id);
	}

	sf::Text modeText;
	modeText.setFont(font);
	modeText.setString(isMinHeap ? "Min Heap" : "Max Heap");
	modeText.setCharacterSize(24);
	// modeText.setFillColor(sf::Color::White);
	modeText.setFillColor(Theme::currentTheme == Theme::ThemeMode::DARK ? Theme::textOnDark() : Theme::textOnLight());
	sf::FloatRect mtb = modeText.getLocalBounds();
	modeText.setOrigin(mtb.left + mtb.width / 2.f, 0.f);
	modeText.setPosition((winW - 350.f) / 2.f, 10);
	window.draw(modeText);

	for (auto& b : buttons) b.draw(window);
	codeViewer.draw(window);

	if (activeDialog) {
		activeDialog->draw(window);
	}
}

void MinMaxHeap::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
	if (activeDialog) {
		activeDialog->handleEvent(event, window);
		return;
	}
	for (auto& b : buttons) b.handleEvent(event, window);
}

void MinMaxHeap::onResize(float w, float h) {
	winW = w;
	winH = h;
	initUI();
	recalculateTargetPositions();
}

void MinMaxHeap::init(const std::vector<int>& data) {
	rawData = data;
	buildHeap();
}

void MinMaxHeap::buildHeap() {
	nodes.clear();
	animSteps.clear();
	animStep = -1;
	codeViewer.setCode({});

	float codeBoxW = 350.f;
	float rootX = 250.f + ((winW - 250.f - codeBoxW) / 2.f);

	for (size_t i = 0; i < rawData.size(); ++i) {
		Node n;
		n.value = rawData[i];
		n.position = sf::Vector2f(rootX, winH + 100.f);
		nodes.push_back(n);
	}
	for (int i = (int)rawData.size() / 2 - 1; i >= 0; --i)
		heapifyDown(i);
	recalculateTargetPositions();
}

void MinMaxHeap::insert(int value) {
	rawData.push_back(value);
	float codeBoxW = 350.f;
	float rootX = 250.f + ((winW - 250.f - codeBoxW) / 2.f);
	Node n;
	n.value = value;
	n.position = sf::Vector2f(rootX, winH + 100.f);
	nodes.push_back(n);
	heapifyUp((int)rawData.size() - 1);
	recalculateTargetPositions();
}

void MinMaxHeap::extract() {
	if (rawData.empty()) return;
	rawData[0] = rawData.back();
	rawData.pop_back();
	nodes[0] = nodes.back();
	nodes.pop_back();
	if (!rawData.empty()) heapifyDown(0);
	recalculateTargetPositions();
}

void MinMaxHeap::recalculateTargetPositions() {
	if (nodes.empty()) return;

	int maxDepth = (int)std::log2(nodes.size());
	float initialSpread = std::max(200.f, std::pow(2.f, (float)maxDepth - 1.f) * 35.f);

	float codeBoxW = 350.f;
	float rootX = 250.f + ((winW - 250.f - codeBoxW) / 2.f);

	std::function<void(int, float, float, float)> calcPos = [&] (int idx, float x, float y, float hSpread) {
		if (idx >= (int)nodes.size()) return;
		nodes[idx].targetPosition = {x, y};
		calcPos(2 * idx + 1, x - hSpread, y + 80.f, hSpread / 2.f);
		calcPos(2 * idx + 2, x + hSpread, y + 80.f, hSpread / 2.f);
	};

	calcPos(0, rootX, 100.f, initialSpread);
}

static bool cmp(int a, int b, bool isMin) { return isMin ? a < b : a > b; }

void MinMaxHeap::heapifyUp(int index) {
	while (index > 0) {
		int parent = (index - 1) / 2;
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
		int left = 2 * index + 1, right = 2 * index + 2, target = index;
		if (left < size && cmp(rawData[left], rawData[target], isMinHeap)) target = left;
		if (right < size && cmp(rawData[right], rawData[target], isMinHeap)) target = right;
		if (target != index) {
			std::swap(rawData[index], rawData[target]);
			std::swap(nodes[index], nodes[target]);
			index = target;
		} else break;
	}
}

void MinMaxHeap::update(float dt) {
	for (auto& n : nodes) {
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

void MinMaxHeap::play() {
	isPlaying = true;
	playTimer = 0.f;
}

void MinMaxHeap::pause() {
	isPlaying = false;
}

void MinMaxHeap::stepForward() {
	if (animStep < 0 || animSteps.empty()) return;
	if (animStep + 1 < (int)animSteps.size()) {
		animStep++;
		if (animStep == (int)animSteps.size() - 1) {
			if (commitOp) {
				commitOp();
				commitOp = nullptr;
			}
			isPlaying = false;
		}
	} else {
		isPlaying = false;
	}
}

void MinMaxHeap::stepBackward() {
	if (animStep > 0) animStep--;
}

void MinMaxHeap::prepareNewOperation() {
	if (commitOp) {
		commitOp();
		commitOp = nullptr;
	}
	animSteps.clear();
	commitOp = nullptr;
	isPlaying = false;
	playTimer = 0.f;
}

void MinMaxHeap::finalizeOperation(std::function<void()> finalCommit) {
	commitOp = finalCommit;
	animStep = 0;
	isPlaying = true;
}

void MinMaxHeap::insertSteps(int value) {
	prepareNewOperation();

	if (!isStepByStep) {
		insert(value);
		VisualStep s;
		s.message = "Inserted " + std::to_string(value) + " (Run at Once)";
		s.highlightColor = sf::Color(0, 140, 85);
		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {
		"void insert(int value) {",
		"    heap.push_back(value);",
		"    int idx = heap.size() - 1;",
		"    while (idx > 0) {",
		"        int parent = (idx - 1) / 2;",
		"        if (cmp(heap[idx], heap[parent])) {",
		"            swap(heap[idx], heap[parent]);",
		"            idx = parent;",
		"        } else",
		"            break;",
		"    }",
		"}"
	};
	codeViewer.setCode(code);

	rawData.push_back(value);
	Node n;
	n.value = value;

	float codeBoxW = 350.f;
	float rootX = 250.f + ((winW - 250.f - codeBoxW) / 2.f);
	n.position = sf::Vector2f(rootX, winH + 100.f);
	nodes.push_back(n);
	recalculateTargetPositions();
	int idx = (int)rawData.size() - 1;

	{
		VisualStep s;
		s.highlighted = {idx};
		s.message = "Placing " + std::to_string(value) + " at position [" + std::to_string(idx) + "]";
		s.highlightColor = sf::Color(80, 200, 255);
		s.highlightedCodeLines = {1, 2};
		animSteps.push_back(s);
	}

	std::vector<int> sim = rawData;
	int si = idx;
	while (si > 0) {
		int parent = (si - 1) / 2;
		bool doSwap = isMinHeap ? sim[si] < sim[parent] : sim[si] > sim[parent];
		VisualStep s;
		s.highlighted = {si, parent};
		if (doSwap) {
			s.highlightedCodeLines = {3, 4, 5, 6, 7};
			s.message = "HeapifyUp: [" + std::to_string(si) + "]=" + std::to_string(sim[si]) + " < parent [" + std::to_string(parent) + "]=" + std::to_string(sim[parent]) + " → Swap";
			animSteps.push_back(s);
			std::swap(sim[si], sim[parent]);
			si = parent;
		} else {
			s.highlightedCodeLines = {3, 4, 5};
			s.message = "HeapifyUp: [" + std::to_string(si) + "]=" + std::to_string(sim[si]) + " OK vs parent [" + std::to_string(parent) + "]=" + std::to_string(sim[parent]) + " → Done";
			s.highlightColor = sf::Color(0, 140, 85);
			animSteps.push_back(s);
			break;
		}
	}

	VisualStep done;
	done.message = "Inserted " + std::to_string(value) + "!";
	done.highlightColor = sf::Color(0, 140, 85);
	done.highlightedCodeLines = {};
	animSteps.push_back(done);

	int insertedIdx = idx;
	finalizeOperation([this, insertedIdx] () {
		heapifyUp(insertedIdx);
		recalculateTargetPositions();
	});
}

void MinMaxHeap::extractSteps() {
	if (rawData.empty()) return;
	prepareNewOperation();

	if (!isStepByStep) {
		extract();
		VisualStep s;
		s.message = "Extracted root (Run at Once)";
		s.highlightColor = sf::Color(220, 60, 60);
		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {
		"int extractRoot() {",
		"    int rootVal = heap[0];",
		"    heap[0] = heap.back();",
		"    heap.pop_back();",
		"    int idx = 0;",
		"    while (true) {",
		"        int target=idx, L=2*idx+1, R=2*idx+2;",
		"        if (L < n && cmp(heap[L], heap[target]))",
		"            target = L;",
		"        if (R < n && cmp(heap[R], heap[target]))",
		"            target = R;",
		"        if (target != idx) {",
		"            swap(heap[idx], heap[target]);",
		"            idx = target;",
		"        } else",
		"            break;",
		"    }",
		"    return rootVal;",
		"}"
	};
	codeViewer.setCode(code);

	int rootVal = rawData[0];
	int lastIdx = (int)rawData.size() - 1;

	{
		VisualStep s;
		s.highlighted = {0};
		s.message = "Extracting root: " + std::to_string(rootVal);
		s.highlightedCodeLines = {1};
		s.highlightColor = sf::Color(220, 60, 60);
		animSteps.push_back(s);
	}

	if (lastIdx > 0) {
		VisualStep s;
		s.highlighted = {0, lastIdx};
		s.highlightedCodeLines = {2, 3, 4};
		s.message = "Moving last element [" + std::to_string(rawData[lastIdx]) + "] to root position";
		animSteps.push_back(s);
	}

	std::vector<int> sim = rawData;
	sim[0] = sim.back();
	sim.pop_back();
	int n = (int)sim.size(), si = 0;
	while (true) {
		int left = 2 * si + 1, right = 2 * si + 2, target = si;
		if (left < n && (isMinHeap ? sim[left] < sim[target] : sim[left] > sim[target])) target = left;
		if (right < n && (isMinHeap ? sim[right] < sim[target] : sim[right] > sim[target])) target = right;
		if (target != si) {
			VisualStep s;
			s.highlighted = {si, target};
			s.highlightedCodeLines = {5, 6, 7, 8, 9, 10, 11, 12, 13};
			s.message = "HeapifyDown: [" + std::to_string(si) + "]=" + std::to_string(sim[si]) + " → swap with [" + std::to_string(target) + "]=" + std::to_string(sim[target]);
			animSteps.push_back(s);
			std::swap(sim[si], sim[target]);
			si = target;
		} else {
			VisualStep s;
			s.highlighted = {si};
			s.highlightedCodeLines = {5, 6, 7, 9, 11};
			s.message = "Heap property satisfied. Done!";
			s.highlightColor = sf::Color(0, 140, 85);
			animSteps.push_back(s);
			break;
		}
	}

	VisualStep done;
	done.message = "Extracted root!";
	done.highlightedCodeLines = {17};
	done.highlightColor = sf::Color(220, 60, 60);
	animSteps.push_back(done);

	finalizeOperation([this] () { extract(); });
}

void MinMaxHeap::searchSteps(int value) {
	prepareNewOperation();
	bool found = false;
	int foundIdx = -1;

	auto canContain = [&] (int val) { return isMinHeap ? (val <= value) : (val >= value); };

	if (!isStepByStep) {
		std::vector<int> stack;
		if (!rawData.empty() && canContain(rawData[0])) stack.push_back(0);
		while (!stack.empty()) {
			int idx = stack.back();
			stack.pop_back();
			if (rawData[idx] == value) {
				found = true;
				foundIdx = idx;
				break;
			}
			int right = 2 * idx + 2;
			int left = 2 * idx + 1;
			if (right < (int)rawData.size() && canContain(rawData[right])) stack.push_back(right);
			if (left < (int)rawData.size() && canContain(rawData[left])) stack.push_back(left);
		}

		VisualStep s;
		if (found) {
			s.message = "Found " + std::to_string(value) + " at index " + std::to_string(foundIdx) + "! (Run at Once)";
			s.highlightColor = sf::Color(0, 140, 85);
			s.highlighted = {foundIdx};
		} else {
			s.message = std::to_string(value) + " not found in the heap. (Run at Once)";
			s.highlightColor = sf::Color(220, 60, 60);
		}
		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {
		"bool search(int value) {",
		"    stack = [0];",
		"    while (!stack.empty()) {",
		"        int idx = stack.pop_back();",
		"        if (heap[idx] == value)",
		"            return true;",
		"        int R = 2*idx+2, L = 2*idx+1;",
		"        if (R < size && canContain(heap[R]))",
		"            stack.push_back(R);",
		"        if (L < size && canContain(heap[L]))",
		"            stack.push_back(L);",
		"    }",
		"    return false;",
		"}"
	};
	codeViewer.setCode(code);

	std::vector<int> stack;
	if (!rawData.empty()) {
		if (canContain(rawData[0])) {
			stack.push_back(0);
		} else {
			VisualStep s;
			s.highlighted = {0};
			s.highlightColor = sf::Color(200, 150, 0);
			s.highlightedCodeLines = {12};
			s.message = "Root is " + std::to_string(rawData[0]) + ", impossible to find " + std::to_string(value) + " below it. Pruned!";
			animSteps.push_back(s);
		}
	}

	while (!stack.empty()) {
		int idx = stack.back();
		stack.pop_back();

		VisualStep s;
		s.highlighted = {idx};
		if (rawData[idx] == value) {
			s.highlightedCodeLines = {2, 3, 4, 5};
			s.message = "Found " + std::to_string(value) + " at index " + std::to_string(idx) + "!";
			s.highlightColor = sf::Color(0, 140, 85);
			animSteps.push_back(s);
			found = true;
			foundIdx = idx;
			break;
		} else {
			s.message = "Searching for " + std::to_string(value) + " - current node is " + std::to_string(rawData[idx]);
			animSteps.push_back(s);
		}

		int right = 2 * idx + 2;
		int left = 2 * idx + 1;

		if (right < (int)rawData.size()) {
			if (canContain(rawData[right])) {
				stack.push_back(right);
			} else {
				VisualStep ps;
				ps.highlighted = {right};
				ps.highlightColor = sf::Color(200, 150, 0);
				ps.message = "Pruning right child " + std::to_string(rawData[right]) + " (impossible to contain " + std::to_string(value) + ")";
				ps.highlightedCodeLines = {7};
				animSteps.push_back(ps);
			}
		}
		if (left < (int)rawData.size()) {
			if (canContain(rawData[left])) {
				stack.push_back(left);
			} else {
				VisualStep ps;
				ps.highlighted = {left};
				ps.highlightColor = sf::Color(200, 150, 0);
				ps.message = "Pruning left child " + std::to_string(rawData[left]) + " (impossible to contain " + std::to_string(value) + ")";
				ps.highlightedCodeLines = {9};
				animSteps.push_back(ps);
			}
		}

		s.highlightedCodeLines = {2, 3, 4, 6, 7, 9};
		animSteps.push_back(s);
	}

	VisualStep done;
	if (found) {
		done.message = "Search finished! Found at index " + std::to_string(foundIdx);
		done.highlightColor = sf::Color(0, 140, 85);
		done.highlighted = {foundIdx};
		done.highlightedCodeLines = {};
	} else {
		VisualStep s;
		s.message = std::to_string(value) + " not found in the heap.";
		s.highlightColor = sf::Color(220, 60, 60);
		s.highlightedCodeLines = {12};
		animSteps.push_back(s);

		done.message = "Search finished! Not found.";
		done.highlightColor = sf::Color(220, 60, 60);
		done.highlightedCodeLines = {};
	}
	animSteps.push_back(done);

	finalizeOperation(nullptr);
}

void MinMaxHeap::deleteAtSteps(int idx) {
	if (idx < 0 || idx >= (int)rawData.size())
		return;

	prepareNewOperation();

	int delVal = rawData[idx];
	int lastIdx = (int)rawData.size() - 1;

	if (!isStepByStep) {
		rawData[idx] = rawData.back();
		rawData.pop_back();
		nodes[idx] = nodes.back();
		nodes.pop_back();
		if (idx < (int)rawData.size()) {
			heapifyDown(idx);
			heapifyUp(idx);
		}
		recalculateTargetPositions();
		VisualStep s;
		s.message = "Deleted element at index " + std::to_string(idx) + " (Run at Once)";
		s.highlightColor = sf::Color(220, 60, 60);
		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {
		"void deleteAt(int idx) {",
		"    heap[idx] = heap.back();",
		"    heap.pop_back();",
		"    heapifyDown(idx);",
		"    heapifyUp(idx);",
		"}"
	};
	codeViewer.setCode(code);

	{
		VisualStep s;
		s.highlighted = {idx};
		s.message = "Deleting element at [" + std::to_string(idx) + "] = " + std::to_string(delVal);
		s.highlightColor = sf::Color(220, 60, 60);
		s.highlightedCodeLines = {0};
		animSteps.push_back(s);
	}

	if (idx != lastIdx) {
		VisualStep s;
		s.highlighted = {idx, lastIdx};
		s.message = "Replace heap[" + std::to_string(idx) + "] with last = " + std::to_string(rawData[lastIdx]);
		s.highlightedCodeLines = {1, 2};
		animSteps.push_back(s);
	}

	std::vector<int> sim = rawData;
	sim[idx] = sim.back();
	sim.pop_back();
	int n = (int)sim.size();

	if (idx < n) {
		int si = idx;
		bool heapifiedDown = false;
		while (true) {
			int left = 2 * si + 1, right = 2 * si + 2, target = si;
			if (left < n && (isMinHeap ? sim[left] < sim[target] : sim[left] > sim[target])) target = left;
			if (right < n && (isMinHeap ? sim[right] < sim[target] : sim[right] > sim[target])) target = right;
			if (target != si) {
				VisualStep s;
				s.highlighted = {si, target};
				s.highlightedCodeLines = {3};
				s.message = "HeapifyDown: [" + std::to_string(si) + "]=" + std::to_string(sim[si]) + " -> swap [" + std::to_string(target) + "]=" + std::to_string(sim[target]);
				animSteps.push_back(s);
				std::swap(sim[si], sim[target]);
				si = target;
				heapifiedDown = true;
			} else break;
		}

		if (!heapifiedDown) {
			si = idx;
			while (si > 0) {
				int parent = (si - 1) / 2;
				if (isMinHeap ? sim[si] < sim[parent] : sim[si] > sim[parent]) {
					VisualStep s;
					s.highlighted = {si, parent};
					s.highlightedCodeLines = {4};
					s.message = "HeapifyUp: [" + std::to_string(si) + "]=" + std::to_string(sim[si]) + " -> swap parent [" + std::to_string(parent) + "]=" + std::to_string(sim[parent]);
					animSteps.push_back(s);
					std::swap(sim[si], sim[parent]);
					si = parent;
				} else break;
			}
		}
	}

	{
		VisualStep done;
		done.message = "Deleted element at index " + std::to_string(idx) + "!";
		done.highlightColor = sf::Color(220, 60, 60);
		animSteps.push_back(done);
	}

	finalizeOperation([this, idx] () {
		if (idx >= (int)rawData.size()) return;
		rawData[idx] = rawData.back();
		rawData.pop_back();
		nodes[idx] = nodes.back();
		nodes.pop_back();
		if (idx < (int)rawData.size()) {
			heapifyDown(idx);
			heapifyUp(idx);
		}
		recalculateTargetPositions();
	});
}

void MinMaxHeap::updateSteps(int idx, int newValue) {
	if (idx < 0 || idx >= (int)rawData.size())
		return;

	prepareNewOperation();

	int oldVal = rawData[idx];

	if (!isStepByStep) {
		rawData[idx] = newValue;
		nodes[idx].value = newValue;
		heapifyDown(idx);
		heapifyUp(idx);
		recalculateTargetPositions();

		VisualStep s;
		s.message = "Updated index " + std::to_string(idx) + " to " + std::to_string(newValue) + " (Run at Once)";
		s.highlightColor = sf::Color(70, 140, 220);
		animSteps.push_back(s);
		animStep = 0;
		return;
	}

	std::vector<std::string> code = {
		"void update(int idx, int newValue) {",
		"    heap[idx] = newValue;",
		"    heapifyDown(idx);",
		"    heapifyUp(idx);",
		"}"
	};
	codeViewer.setCode(code);

	{
		VisualStep s;
		s.highlighted = {idx};
		s.message = "Changing heap[" + std::to_string(idx) + "] from " + std::to_string(oldVal) + " to " + std::to_string(newValue);
		s.highlightColor = sf::Color(70, 140, 220);
		s.highlightedCodeLines = {1};
		animSteps.push_back(s);
	}

	std::vector<int> sim = rawData;
	sim[idx] = newValue;
	int n = (int)sim.size();

	int si = idx;
	bool heapifiedDown = false;
	while (true) {
		int left = 2 * si + 1, right = 2 * si + 2, target = si;
		if (left < n && (isMinHeap ? sim[left] < sim[target] : sim[left] > sim[target])) target = left;
		if (right < n && (isMinHeap ? sim[right] < sim[target] : sim[right] > sim[target])) target = right;
		if (target != si) {
			VisualStep s;
			s.highlighted = {si, target};
			s.highlightedCodeLines = {2};
			s.message = "HeapifyDown: [" + std::to_string(si) + "]=" + std::to_string(sim[si]) + " -> swap [" + std::to_string(target) + "]=" + std::to_string(sim[target]);
			animSteps.push_back(s);
			std::swap(sim[si], sim[target]);
			si = target;
			heapifiedDown = true;
		} else break;
	}

	if (!heapifiedDown) {
		si = idx;
		while (si > 0) {
			int parent = (si - 1) / 2;
			if (isMinHeap ? sim[si] < sim[parent] : sim[si] > sim[parent]) {
				VisualStep s;
				s.highlighted = {si, parent};
				s.highlightedCodeLines = {3};
				s.message = "HeapifyUp: [" + std::to_string(si) + "]=" + std::to_string(sim[si]) + " -> swap parent [" + std::to_string(parent) + "]=" + std::to_string(sim[parent]);
				animSteps.push_back(s);
				std::swap(sim[si], sim[parent]);
				si = parent;
			} else break;
		}
	}

	{
		VisualStep done;
		done.message = "Update completed!";
		done.highlightColor = sf::Color(70, 140, 220);
		animSteps.push_back(done);
	}

	finalizeOperation([this, idx, newValue] () {
		if (idx >= (int)rawData.size()) return;
		rawData[idx] = newValue;
		nodes[idx].value = newValue;
		heapifyDown(idx);
		heapifyUp(idx);
		recalculateTargetPositions();
	});
}
