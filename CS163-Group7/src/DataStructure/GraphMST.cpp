#include "DataStructure/GraphMST.h"
#include "UI/Theme.h"
#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <fstream>

GraphMST::GraphMST(float windowWidth, float windowHeight, AlgoMode mode)
	: kruskalStep(0), kruskalRunning(false), kruskalDone(false),
	  primStep(0), primRunning(false), primDone(false),
	  totalMSTWeight(0), currentMode(mode) {
	winW = windowWidth;
	winH = windowHeight;

	if (!font.loadFromFile("assets/fonts/arial.ttf"))
		std::cerr << "Failed to load font\n";

	initUI();
}

GraphMST::~GraphMST() {}

void GraphMST::initUI() {
	buttons.clear();

	float codeBoxW = 350.f;
	codeViewer.init(winW - codeBoxW, 0, codeBoxW, winH * 0.55f, &font);

	float btnW = 260.f;
	float btnH = 45.f;
	float gap = 10.f;
	int maxCols = std::max(1, (int)((winW - codeBoxW - 480.f) / (btnW + gap)));
	float startX = winW - codeBoxW - (maxCols * btnW + (maxCols - 1) * gap) - 15.f;
	float colX = startX;
	float startY = winH - 150.f;
	int colCount = 0;

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Add Node", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Add Node", std::vector<InputDialog::Field>{{"Node ID:", "ID"}}, font, [this] (const std::vector<std::string>& results) {
			if (!results[0].empty())
				addNode(std::stoi(results[0]));
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

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Add Edge", font, [this, colX, btnW, startY] () {
		activeDialog = std::make_unique<InputDialog>("Add Edge", std::vector<InputDialog::Field>{{"From Node:", "From"}, {"To Node:", "To"}, {"Weight:", "Wt"}}, font, [this] (const std::vector<std::string>& results) {
			if (!results[0].empty() && !results[1].empty() && !results[2].empty())
				addEdge(std::stoi(results[0]), std::stoi(results[1]), std::stoi(results[2]));
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

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Random Graph", font, [this] () {
		clear();
		std::ifstream fin("assets/graphs/graph.txt");
		if (false && fin.good()) {
			int numNodes, numEdges;
			fin >> numNodes >> numEdges;
			std::cout << "Loaded graph with " << numNodes << " nodes and " << numEdges << " edges.\n";
			for (int i = 0; i < numNodes; ++i) addNode(i);
			for (int i = 0; i < numEdges; ++i) {
				int from, to, weight;
				fin >> from >> to >> weight;
				addEdge(from, to, weight);
			}
		} else {
			int n = 6;
			for (int i = 0; i < n; ++i) addNode(i);

			int m = 10;
			for (int i = 0; i < m; ++i) {
				int from = rand() % n;
				int to = rand() % n;
				if (from == to) continue;
				int weight = 1 + rand() % 20;
				addEdge(from, to, weight);
			}
		}

		kruskalRunning = false;
		kruskalDone = false;
		primRunning = false;
		primDone = false;
		kruskalStep = 0;
		primStep = 0;

		for (auto& e : edges) {
			e.inMST = false;
			e.rejected = false;
		}
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), (currentMode == GraphMST::AlgoMode::KRUSKAL ? "Mode: Kruskal" : "Mode: Prim"), font, [this] () {
		setAlgoMode(currentMode == GraphMST::AlgoMode::KRUSKAL ? GraphMST::AlgoMode::PRIM : GraphMST::AlgoMode::KRUSKAL);
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), (currentMode == GraphMST::AlgoMode::KRUSKAL ? "Run Kruskal" : "Run Prim"), font, [this] () {
		if (currentMode == GraphMST::AlgoMode::KRUSKAL)
			runKruskal();
		else
			runPrim();
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Step", font, [this] () {
		stepForward();
	}));

	colCount++;
	if (colCount >= maxCols) {
		colCount = 0;
		colX = startX;
		startY += btnH + gap;
	} else {
		colX += btnW + gap;
	}

	buttons.push_back(Button(sf::Vector2f(btnW, btnH), sf::Vector2f(colX, startY), "Clear", font, [this] () {
		clear();
	}));
}

void GraphMST::drawArrow(sf::RenderWindow& window, sf::Vector2f from, sf::Vector2f to, sf::Color color, int weight, bool inMST) {
	sf::Vector2f dir = to - from;
	float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);

	if (len < 1.f)
		return;

	dir /= len;

	float nodeR = 20.f;
	sf::Vector2f s = from + dir * nodeR;
	sf::Vector2f e = to - dir * nodeR;

	float lineLen = len - 2.f * nodeR;

	if (lineLen < 1.f)
		return;

	float thickness = inMST ? 4.f : 2.f;
	sf::RectangleShape line(sf::Vector2f(lineLen, thickness));
	line.setPosition(s);
	line.setFillColor(color);
	float angle = std::atan2(dir.y, dir.x) * 180.f / 3.14159f;
	line.setRotation(angle);
	window.draw(line);

	sf::Vector2f mid = (from + to) / 2.f;
	sf::Text wText;
	wText.setFont(font);
	wText.setString(std::to_string(weight));
	wText.setCharacterSize(14);
	wText.setFillColor(sf::Color(150, 150, 150));
	sf::FloatRect wb = wText.getLocalBounds();
	wText.setOrigin(wb.left + wb.width / 2.f, wb.top + wb.height / 2.f);
	wText.setPosition(mid + sf::Vector2f(-dir.y, dir.x) * 14.f);
	window.draw(wText);
}

void GraphMST::draw(sf::RenderWindow& window) {
	float nodeR = 20.f;
	float centerX = (winW - 350.f) / 2.f;

	sf::Text title;
	title.setFont(font);
	title.setString(currentMode == GraphMST::AlgoMode::KRUSKAL ? "MST - Kruskal's Algorithm" : "MST - Prim's Algorithm");
	title.setCharacterSize(24);
	title.setFillColor(Theme::currentTheme == Theme::ThemeMode::DARK ? Theme::textOnDark() : Theme::textOnLight());
	sf::FloatRect titleBounds = title.getLocalBounds();
	title.setOrigin(titleBounds.left + titleBounds.width / 2.f, 0.f);
	title.setPosition(centerX, 10);
	window.draw(title);

	sf::Text guide;
	guide.setFont(font);
	guide.setString("Left drag: move node | Right click node: lock/unlock");
	guide.setCharacterSize(14);
	guide.setFillColor(sf::Color(150, 150, 150));
	sf::FloatRect guideBounds = guide.getLocalBounds();
	guide.setOrigin(guideBounds.left + guideBounds.width / 2.f, 0.f);
	guide.setPosition(centerX, 70);
	window.draw(guide);

	if (kruskalDone || primDone) {
		sf::Text status;
		status.setFont(font);
		status.setString("MST Complete! Total Weight: " + std::to_string(totalMSTWeight));
		status.setCharacterSize(18);
		// status.setFillColor(sf::Color(100, 255, 150));
		status.setFillColor(sf::Color(0, 170, 120));
		sf::FloatRect stBounds = status.getLocalBounds();
		status.setOrigin(stBounds.left + stBounds.width / 2.f, 0.f);
		status.setPosition(centerX, 40);
		window.draw(status);
	} else if (kruskalRunning || primRunning) {
		sf::Text status;
		status.setFont(font);
		int currentStep = kruskalRunning ? kruskalStep : primStep;
		status.setString("Step " + std::to_string(currentStep) + "  |  Press 'Step' to advance");
		status.setCharacterSize(16);
		status.setFillColor(sf::Color(70, 150, 220));
		sf::FloatRect stBounds = status.getLocalBounds();
		status.setOrigin(stBounds.left + stBounds.width / 2.f, 0.f);
		status.setPosition(centerX, 40);
		window.draw(status);
	}

	if (!highlightedCodeLines.empty())
		codeViewer.setActiveLines(highlightedCodeLines);
	else
		codeViewer.setActiveLines({});

	for (auto& e : edges) {
		int fi = findNode(e.from);
		int ti = findNode(e.to);
		if (fi < 0 || ti < 0) continue;

		sf::Color edgeColor(150, 150, 150);
		bool inMST = false;
		if (e.inMST) {
			edgeColor = sf::Color(80, 220, 100);
			inMST = true;
		} else if (e.rejected) {
			edgeColor = sf::Color(200, 80, 80);
		}

		drawArrow(window, nodes[fi].position + sf::Vector2f(nodeR, nodeR),
				  nodes[ti].position + sf::Vector2f(nodeR, nodeR), edgeColor, e.weight, inMST);
	}

	for (size_t i = 0; i < nodes.size(); ++i) {
		auto& n = nodes[i];
		sf::CircleShape circle(nodeR);
		circle.setPosition(n.position);

		if (primRunning || primDone) {
			if (i < primVisited.size() && primVisited[i])
				circle.setFillColor(sf::Color(100, 180, 100));
			else
				circle.setFillColor(sf::Color(40, 100, 180));
		} else {
			circle.setFillColor(sf::Color(40, 100, 180));
		}

		circle.setOutlineThickness(3.f);
		circle.setOutlineColor(n.locked ? sf::Color(230, 140, 40) : sf::Color(50, 140, 210));
		window.draw(circle);

		sf::Text idText;
		idText.setFont(font);
		idText.setString(std::to_string(n.id));
		idText.setCharacterSize(16);
		idText.setFillColor(sf::Color::White);
		sf::FloatRect tb = idText.getLocalBounds();
		idText.setOrigin(tb.left + tb.width / 2.f, tb.top + tb.height / 2.f);
		idText.setPosition(n.position + sf::Vector2f(nodeR, nodeR));
		window.draw(idText);

		if (n.locked) {
			sf::Text lockText;
			lockText.setFont(font);
			lockText.setString("L");
			lockText.setCharacterSize(12);
			lockText.setFillColor(sf::Color(255, 220, 120));
			lockText.setPosition(n.position.x + nodeR + 12.f, n.position.y - 8.f);
			window.draw(lockText);
		}
	}

	float listX = winW - 350.f;
	float listY = winH * 0.55f;
	float bottomH = winH - listY;

	sf::RectangleShape bottomPanel(sf::Vector2f(350.f, bottomH));
	bottomPanel.setPosition(listX, listY);
	bottomPanel.setFillColor(sf::Color(30, 30, 30));
	bottomPanel.setOutlineThickness(1.f);
	bottomPanel.setOutlineColor(sf::Color(64, 64, 64));
	window.draw(bottomPanel);

	sf::Text header;
	header.setFont(font);
	header.setString("Selected Edges (MST):");
	header.setCharacterSize(16);
	header.setFillColor(sf::Color::White);
	header.setPosition(listX + 10.f, listY + 10.f);
	window.draw(header);

	sf::RectangleShape sep2(sf::Vector2f(330.f, 2.f));
	sep2.setPosition(listX + 10.f, listY + 35.f);
	sep2.setFillColor(sf::Color(80, 80, 80));
	window.draw(sep2);

	int count = 0;
	for (const auto& e : chosenEdges) {
		std::string label = "Edge: " + std::to_string(e.from) + " - " + std::to_string(e.to) + "   (weight: " + std::to_string(e.weight) + ")";
		sf::Text et;
		et.setFont(font);
		et.setString(label);
		et.setCharacterSize(14);
		et.setFillColor(sf::Color(80, 220, 100));
		et.setPosition(listX + 15.f, listY + 45.f + count * 22.f);
		window.draw(et);
		count++;
	}

	for (auto& btn : buttons) btn.draw(window);
	codeViewer.draw(window);

	if (activeDialog) {
		activeDialog->draw(window);
	}
}

void GraphMST::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
	if (activeDialog) {
		activeDialog->handleEvent(event, window);
		return;
	}
	for (auto& btn : buttons) btn.handleEvent(event, window);

	const float nodeR = 20.f;
	if (event.type == sf::Event::MouseButtonPressed) {
		sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y));
		for (int i = (int)nodes.size() - 1; i >= 0; --i) {
			sf::Vector2f center = nodes[i].position + sf::Vector2f(nodeR, nodeR);
			sf::Vector2f d = mousePos - center;
			float dist2 = d.x * d.x + d.y * d.y;
			if (dist2 <= nodeR * nodeR) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					draggedNodeIndex = i;
					dragOffset = nodes[i].position - mousePos;
				} else if (event.mouseButton.button == sf::Mouse::Right) {
					nodes[i].locked = !nodes[i].locked;
				}
				break;
			}
		}
	} else if (event.type == sf::Event::MouseMoved) {
		if (draggedNodeIndex >= 0 && draggedNodeIndex < (int)nodes.size()) {
			sf::Vector2f mousePos = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y));
			nodes[draggedNodeIndex].position = mousePos + dragOffset;
			clampNodeToGraphArea(nodes[draggedNodeIndex].position);
		}
	} else if (event.type == sf::Event::MouseButtonReleased) {
		if (event.mouseButton.button == sf::Mouse::Left)
			draggedNodeIndex = -1;
	}
}

void GraphMST::onResize(float w, float h) {
	winW = w;
	winH = h;
	initUI();
	for (auto& n : nodes)
		clampNodeToGraphArea(n.position);
}

void GraphMST::UnionFind::init(int n) {
	parent.resize(n);
	rank_.resize(n, 0);

	for (int i = 0; i < n; ++i)
		parent[i] = i;
}

int GraphMST::UnionFind::find(int x) {
	if (parent[x] != x)
		parent[x] = find(parent[x]);

	return parent[x];
}

bool GraphMST::UnionFind::unite(int x, int y) {
	int px = find(x);
	int py = find(y);

	if (px == py)
		return false;

	if (rank_[px] < rank_[py])
		std::swap(px, py);

	parent[py] = px;

	if (rank_[px] == rank_[py])
		rank_[px]++;

	return true;
}

void GraphMST::addNode(int id) {
	if (findNode(id) >= 0)
		return;

	GNode n;
	n.id = id;

	float codeBoxW = 350.f;
	float realAvailW = winW - 250.f - codeBoxW;
	float cx = 250.f + (realAvailW / 2.f);
	float cy = winH / 2.f - 40.f;

	float r = std::min(realAvailW / 3.f, (winH - 200.f) / 3.f);
	float angle = (float)nodes.size() * 1.2f;

	n.position = sf::Vector2f(cx + r * std::cos(angle), cy + r * std::sin(angle));
	n.locked = false;
	clampNodeToGraphArea(n.position);
	nodes.push_back(n);
}

void GraphMST::addEdge(int from, int to, int weight) {
	if (findNode(from) < 0 || findNode(to) < 0)
		return;

	edges.push_back({from, to, weight, false, false});
}

void GraphMST::clear() {
	nodes.clear();
	edges.clear();
	sortedEdges.clear();
	chosenEdges.clear();
	primEdges = std::priority_queue<Edge, std::vector<Edge>, CompareEdge>();

	kruskalRunning = false;
	kruskalDone = false;
	primRunning = false;
	primDone = false;

	kruskalStep = 0;
	primStep = 0;

	totalMSTWeight = 0;
	draggedNodeIndex = -1;
}

void GraphMST::setAlgoMode(AlgoMode mode) {
	currentMode = mode;
	kruskalRunning = false;
	kruskalDone = false;
	primRunning = false;
	primDone = false;
	isPlaying = false;
	kruskalStep = 0;
	primStep = 0;
	totalMSTWeight = 0;
	sortedEdges.clear();
	chosenEdges.clear();
	primVisited.clear();
	primEdges = std::priority_queue<Edge, std::vector<Edge>, CompareEdge>();

	for (auto& e : edges) {
		e.inMST = false;
		e.rejected = false;
	}

	if (buttons.size() > 4) {
		buttons[3].setText(currentMode == AlgoMode::KRUSKAL ? "Mode: Kruskal" : "Mode: Prim");
		buttons[4].setText(currentMode == AlgoMode::KRUSKAL ? "Run Kruskal" : "Run Prim");
	}
}

void GraphMST::runKruskal() {
	for (auto& e : edges) {
		e.inMST = false;
		e.rejected = false;
	}

	totalMSTWeight = 0;
	sortedEdges = edges;
	chosenEdges.clear();

	std::sort(sortedEdges.begin(), sortedEdges.end(), [] (const Edge& a, const Edge& b) {
		return a.weight < b.weight;
	});

	uf.init((int)nodes.size());

	kruskalStep = 0;
	kruskalRunning = true;
	kruskalDone = false;
	primRunning = false;

	highlightedCodeLines = {1};

	std::vector<std::string> kc = {
		"void kruskal() {",
		"    sortEdges();",
		"    for (Edge e : edges) {",
		"        if (find(u) != find(v)) {",
		"            union(u, v);",
		"            addToMST(e);",
		"        }",
		"    }",
		"}"
	};
	codeViewer.setCode(kc);

	if (!isStepByStep) {
		while (!kruskalDone)
			stepForward();

		kruskalRunning = false;
		highlightedCodeLines = {};
	} else {
		isPlaying = true;
	}
}

void GraphMST::runPrim() {
	for (auto& e : edges) {
		e.inMST = false;
		e.rejected = false;
	}

	totalMSTWeight = 0;
	chosenEdges.clear();

	if (nodes.empty())
		return;

	primVisited.assign(nodes.size(), false);
	primEdges = std::priority_queue<Edge, std::vector<Edge>, CompareEdge>();

	primVisited[0] = true;

	for (const auto& e : edges)
		if (findNode(e.from) == 0 || findNode(e.to) == 0)
			primEdges.push(e);

	primStep = 0;
	primRunning = true;
	primDone = false;
	kruskalRunning = false;

	highlightedCodeLines = {1};

	std::vector<std::string> pc = {
		"void prim() {",
		"    visited[node] = true;",
		"    pushEdges(node);",
		"    while (!pq.empty()) {",
		"        e = pq.top(); pq.pop();",
		"        u = e.to;",
		"        w = e.weight;",
		"        if (visited[u]) ",
		"            continue;",
		"        visited[u] = true;",
		"        totalMSTWeight += w;",
		"        addToMST(e);",
		"        pushEdges(u);",
		"    }",
		"}"
	};
	codeViewer.setCode(pc);

	if (!isStepByStep) {
		while (!primDone)
			stepForward();

		primRunning = false;
		highlightedCodeLines = {};
	} else {
		isPlaying = true;
	}
}

int GraphMST::findNode(int id) {
	for (int i = 0; i < (int)nodes.size(); ++i)
		if (nodes[i].id == id)
			return i;

	return -1;
}

void GraphMST::clampNodeToGraphArea(sf::Vector2f& pos) const {
	float codeBoxW = 350.f;
	float realAvailW = winW - 250.f - codeBoxW;
	float minX = 280.f;
	float maxX = 250.f + realAvailW - 40.f;
	float minY = 60.f;
	float maxY = winH - 200.f;

	if (pos.x < minX) pos.x = minX;
	if (pos.x > maxX) pos.x = maxX;
	if (pos.y < minY) pos.y = minY;
	if (pos.y > maxY) pos.y = maxY;
}

void GraphMST::applyForceLayout(float dt) {
	const float kCoulomb = 60000.f;
	const float minDist = 200.f;
	const float maxRepel = 6000.f;
	const float springK = 0.024f;
	const float restLength = 500.f;

	for (size_t i = 0; i < nodes.size(); ++i) {
		if (nodes[i].locked || (int)i == draggedNodeIndex)
			continue;

		sf::Vector2f force(0, 0);

		for (size_t j = 0; j < nodes.size(); ++j) {
			if (i == j)
				continue;

			sf::Vector2f diff = nodes[i].position - nodes[j].position;
			float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);

			if (dist < minDist)
				dist = minDist;

			sf::Vector2f dir = diff / dist;
			float repel = kCoulomb / (dist * dist);
			if (repel > maxRepel)
				repel = maxRepel;
			force += dir * repel;
		}

		for (auto& e : edges) {
			if (e.from == nodes[i].id) {
				int ti = findNode(e.to);

				if (ti >= 0) {
					sf::Vector2f diff = nodes[ti].position - nodes[i].position;
					float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
					if (dist > 1.f) {
						sf::Vector2f dir = diff / dist;
						force += dir * (dist - restLength) * springK;
					}
				}
			}

			if (e.to == nodes[i].id) {
				int fi = findNode(e.from);

				if (fi >= 0) {
					sf::Vector2f diff = nodes[fi].position - nodes[i].position;
					float dist = std::sqrt(diff.x * diff.x + diff.y * diff.y);
					if (dist > 1.f) {
						sf::Vector2f dir = diff / dist;
						force += dir * (dist - restLength) * springK;
					}
				}
			}
		}

		float codeBoxW = 350.f;
		float realAvailW = winW - 250.f - codeBoxW;
		float cx = 250.f + (realAvailW / 2.f);
		float cy = winH / 2.f - 40.f;

		sf::Vector2f center(cx, cy);
		force += (center - nodes[i].position) * 0.05f;

		nodes[i].position += force * dt;

		clampNodeToGraphArea(nodes[i].position);
	}
}

void GraphMST::update(float dt) {
	applyForceLayout(dt);

	if (isPlaying && (kruskalRunning || primRunning)) {
		playTimer += dt;

		if (playTimer >= playInterval) {
			playTimer = 0.f;
			stepForward();
		}
	}

	if (!kruskalRunning && !primRunning)
		isPlaying = false;
}

void GraphMST::play() {
	isPlaying = true;
	playTimer = 0.f;

	if (!kruskalRunning && !primRunning) {
		if (currentMode == GraphMST::AlgoMode::KRUSKAL)
			runKruskal();
		else
			runPrim();
	}
}

void GraphMST::pause() {
	isPlaying = false;
}

void GraphMST::stepForward() {
	if (kruskalRunning && !kruskalDone) {
		if (kruskalStep >= (int)sortedEdges.size()) {
			kruskalDone = true;
			kruskalRunning = false;
			return;
		}

		auto& se = sortedEdges[kruskalStep];
		int fi = findNode(se.from);
		int ti = findNode(se.to);

		if (fi >= 0 && ti >= 0 && uf.unite(fi, ti)) {
			highlightedCodeLines = {3, 4, 5, 6};
			se.inMST = true;
			totalMSTWeight += se.weight;

			for (auto& e : edges) {
				if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
					e.inMST = true;
					chosenEdges.push_back(e);
					break;
				}
			}
		} else {
			highlightedCodeLines = {3};
			se.rejected = true;

			for (auto& e : edges) {
				if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
					e.rejected = true;
					break;
				}
			}
		}

		kruskalStep++;

		if (kruskalStep >= (int)sortedEdges.size()) {
			kruskalDone = true;
			kruskalRunning = false;
			highlightedCodeLines = {};
		}
	} else if (primRunning && !primDone) {
		highlightedCodeLines = {3};

		if (primEdges.empty()) {
			primDone = true;
			primRunning = false;
			highlightedCodeLines = {};
			return;
		}

		Edge se = primEdges.top();
		primEdges.pop();

		int fi = findNode(se.from);
		int ti = findNode(se.to);

		if (fi >= 0 && ti >= 0 && primVisited[fi] && primVisited[ti]) {
			highlightedCodeLines = {4, 5, 6, 7, 8};

			for (auto& e : edges) {
				if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
					e.rejected = true;
					break;
				}
			}
			return;
		}

		if (fi >= 0 && ti >= 0 && (primVisited[fi] ^ primVisited[ti])) {
			highlightedCodeLines = {4, 5, 6, 7, 8, 9, 10, 11, 12};
			totalMSTWeight += se.weight;

			int newVisitedIdx = primVisited[fi] ? ti : fi;
			primVisited[newVisitedIdx] = true;

			for (auto& e : edges) {
				if (e.from == se.from && e.to == se.to && e.weight == se.weight) {
					e.inMST = true;
					chosenEdges.push_back(e);
					break;
				}
			}

			for (const auto& e : edges) {
				int ef = findNode(e.from);
				int et = findNode(e.to);

				if ((ef == newVisitedIdx || et == newVisitedIdx) && !(primVisited[ef] && primVisited[et]))
					primEdges.push(e);
			}
		}

		primStep++;

		if (primEdges.empty()) {
			primDone = true;
			primRunning = false;
			highlightedCodeLines = {};
		}
	}
}

void GraphMST::stepBackward() {}
