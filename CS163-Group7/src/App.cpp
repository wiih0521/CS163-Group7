#include "App.h"
#include "DataStructure/AVLTree.h"
#include "DataStructure/GraphMST.h"
#include "DataStructure/MinMaxHeap.h"
#include "DataStructure/SinglyLinkedList.h"
#include "UI/Theme.h"
#include <algorithm>
#include <iostream>

#ifdef _WIN32
#include <windows.h>
#endif

namespace Theme {
ThemeMode currentTheme = ThemeMode::DARK;
}

App::App() : window(sf::VideoMode(1280, 720), "Data Structure Visualizer", sf::Style::Default) {
#ifdef _WIN32
	HWND hwnd = window.getSystemHandle();
	ShowWindow(hwnd, SW_MAXIMIZE);
#endif

	windowWidth = (float)window.getSize().x;
	windowHeight = (float)window.getSize().y;

	window.setFramerateLimit(60);
	window.setMouseCursorVisible(true);

	activeDS = DSType::NONE;
	currentDS = nullptr;
	isStepMode = true;
	currentState = AppState::MAIN_MENU;

	initUI();
}

App::~App() {}

void App::initUI() {
	if (!font.loadFromFile("assets/fonts/arial.ttf"))
		std::cerr << "Error loading font for App UI\n";

	controlButtons.clear();
	exitConfirmButtons.clear();

	buildMainMenu();

	std::string themeButtonText = (Theme::currentTheme == Theme::ThemeMode::DARK) ? "Dark" : "Light";
	themeToggleButton = std::make_unique<Button>(
		sf::Vector2f(100, 50), sf::Vector2f(20.f, 20.f), themeButtonText, font, [this] () {
			Theme::currentTheme = (Theme::currentTheme == Theme::ThemeMode::DARK) 
				? Theme::ThemeMode::LIGHT 
				: Theme::ThemeMode::DARK;
			uiNeedsUpdate = true;
		});

	backButton = std::make_unique<Button>(
		sf::Vector2f(100, 50), sf::Vector2f(20.f, 90.f), "Back", font, [this] () {
			currentState = AppState::MAIN_MENU;
			currentDS = nullptr;
			activeDS = DSType::NONE;
		});

	const float arrowW = 72.f;
	const float modeW = 260.f;
	const float btnH = 45.f;
	const float gap = 10.f;
	const float controlsY = windowHeight - 150.f;

	const float leftControlsX = 20.f;

	const float arrow1X = leftControlsX + gap;
	const float modeX = arrow1X + arrowW + gap;
	const float arrow2X = modeX + modeW + gap;

	controlButtons.push_back(Button(sf::Vector2f(arrowW, btnH), sf::Vector2f(arrow1X, controlsY), "<<", font, [this] () {
		if (currentDS)
			currentDS->stepBackward();
	}));

	controlButtons.push_back(Button(sf::Vector2f(arrowW, btnH), sf::Vector2f(arrow2X, controlsY), ">>", font, [this] () {
		if (currentDS)
			currentDS->stepForward();
	}));

	controlButtons.push_back(Button(sf::Vector2f(modeW, btnH), sf::Vector2f(modeX, controlsY), isStepMode ? "Step by step" : "Run at once", font, [this] () {
		isStepMode = !isStepMode;
		controlButtons[2].setText(isStepMode ? "Step by step" : "Run at once");
		controlButtons[0].setEnabled(isStepMode);
		controlButtons[1].setEnabled(isStepMode);
		if (currentDS)
			currentDS->setStepMode(isStepMode);
	}));

	float row3Y = controlsY + btnH + 15.f;

	const float sliderW = 400.f;
	speedSlider = std::make_unique<Slider>(sf::Vector2f(sliderW, 7), sf::Vector2f(leftControlsX, row3Y + 15.f), 0.1f, 2.0f, 0.1f, font, "Animation Speed");
	speedSlider->setOnValueChange([this] (float val) {
		if (currentDS)
			currentDS->setPlayInterval(val); 
	});

	controlButtons[0].setEnabled(isStepMode);
	controlButtons[1].setEnabled(isStepMode);
}

void App::buildMainMenu() {
	mainMenuButtons.clear();
	float boxW = 350.f;
	float boxH = 180.f;
	float gap = 50.f;

	float startX = (windowWidth - (2 * boxW + gap)) / 2.f;
	float startY = (windowHeight - (2 * boxH + gap)) / 2.f;

	mainMenuButtons.push_back(Button(sf::Vector2f(boxW, boxH), sf::Vector2f(startX, startY), "Singly Linked List", font, [this] () {
		activeDS = DSType::SINGLY_LINKED_LIST;
		currentDS = std::make_unique<SinglyLinkedList>(windowWidth, windowHeight);
		currentDS->setStepMode(isStepMode);
		if (speedSlider) currentDS->setPlayInterval(speedSlider->getValue());
		currentState = AppState::VISUALIZER;
	}));

	mainMenuButtons.push_back(Button(sf::Vector2f(boxW, boxH), sf::Vector2f(startX + boxW + gap, startY), "AVL Tree", font, [this] () {
		activeDS = DSType::AVL_TREE;
		currentDS = std::make_unique<AVLTree>(windowWidth, windowHeight);
		currentDS->setStepMode(isStepMode);
		if (speedSlider) currentDS->setPlayInterval(speedSlider->getValue());
		currentState = AppState::VISUALIZER;
	}));

	mainMenuButtons.push_back(Button(sf::Vector2f(boxW, boxH), sf::Vector2f(startX, startY + boxH + gap), "Min/Max Heap", font, [this] () {
		activeDS = DSType::MIN_MAX_HEAP;
		currentDS = std::make_unique<MinMaxHeap>(windowWidth, windowHeight);
		currentDS->setStepMode(isStepMode);
		if (speedSlider) currentDS->setPlayInterval(speedSlider->getValue());
		currentState = AppState::VISUALIZER;
	}));

	mainMenuButtons.push_back(Button(sf::Vector2f(boxW, boxH), sf::Vector2f(startX + boxW + gap, startY + boxH + gap), "Spanning Tree (MST)", font, [this] () {
		activeDS = DSType::KRUSKAL_MST;
		currentDS = std::make_unique<GraphMST>(windowWidth, windowHeight, GraphMST::AlgoMode::KRUSKAL);
		currentDS->setStepMode(isStepMode);
		if (speedSlider) currentDS->setPlayInterval(speedSlider->getValue());
		currentState = AppState::VISUALIZER;
	}));

	if (themeToggleButton) {
		std::string themeButtonText = (Theme::currentTheme == Theme::ThemeMode::DARK) ? "Dark" : "Light";
		themeToggleButton->setText(themeButtonText);
	}
}

void App::run() {
	while (window.isOpen()) {
		processEvents();
		update(deltaClock.restart().asSeconds());
		render();
	}
}

void App::processEvents() {
	sf::Event event;

	while (window.pollEvent(event)) {
		if (event.type == sf::Event::Closed)
			window.close();

		if (event.type == sf::Event::Resized) {
			sf::FloatRect visibleArea(0, 0, (float)event.size.width, (float)event.size.height);
			window.setView(sf::View(visibleArea));

			windowWidth = (float)event.size.width;
			windowHeight = (float)event.size.height;

			initUI();

			if (currentDS)
				currentDS->onResize(windowWidth, windowHeight);

			window.setMouseCursorVisible(true);
		}

		if (event.type == sf::Event::GainedFocus)
			window.setMouseCursorVisible(true);

		if (showExitConfirm) {
			for (auto& btn : exitConfirmButtons)
				btn.handleEvent(event, window);
			continue;
		}

		if (themeToggleButton)
			themeToggleButton->handleEvent(event, window);

		if (currentState == AppState::MAIN_MENU) {
			for (auto& btn : mainMenuButtons)
				btn.handleEvent(event, window);
		} else {
			if (event.type == sf::Event::KeyPressed && currentDS) {
				if (isStepMode && event.key.code == sf::Keyboard::Left) {
					currentDS->stepBackward();
				} else if (isStepMode && event.key.code == sf::Keyboard::Right) {
					currentDS->stepForward();
				} else if (isStepMode && event.key.shift && event.key.code == sf::Keyboard::Comma) {
					currentDS->stepBackward();
				} else if (isStepMode && event.key.shift && event.key.code == sf::Keyboard::Period) {
					currentDS->stepForward();
				}
			}
			if (backButton)
				backButton->handleEvent(event, window);
			for (std::size_t i = 0; i < controlButtons.size(); ++i)
				controlButtons[i].handleEvent(event, window);
			if (speedSlider)
				speedSlider->handleEvent(event, window);
			if (currentDS)
				currentDS->handleEvent(event, window);
		}
	}
}

void App::update(float dt) {
	if (dt > 0.05f)
		dt = 0.05f;

	if (uiNeedsUpdate) {
		buildMainMenu();
		uiNeedsUpdate = false;
	}

	if (currentDS)
		currentDS->update(dt);
}

void App::render() {
	window.clear(Theme::background());

	if (currentState == AppState::MAIN_MENU) {
		sf::Text title("Data Structure Visualizer\n\n             Group 7", font, 47);
		title.setFillColor(Theme::currentTheme == Theme::ThemeMode::DARK ? Theme::textOnDark() : Theme::textOnLight());
		sf::FloatRect tb = title.getLocalBounds();
		title.setPosition((windowWidth - tb.width) / 2.f, 100.f);
		window.draw(title);

		for (auto& btn : mainMenuButtons)
			btn.draw(window);

		
		if (themeToggleButton)
			themeToggleButton->draw(window);
	} else {
		if (currentDS)
			currentDS->draw(window);

		if (backButton)
			backButton->draw(window);

		
		if (themeToggleButton)
			themeToggleButton->draw(window);
	

		sf::Text controlsTitle("Controls", font, 20);
		controlsTitle.setPosition(10.f, windowHeight - 185.f);
		controlsTitle.setFillColor(Theme::currentTheme == Theme::ThemeMode::DARK ? Theme::textOnDark() : Theme::textOnLight());
		window.draw(controlsTitle);

		for (std::size_t i = 0; i < controlButtons.size(); ++i)
			controlButtons[i].draw(window);

		if (speedSlider)
			speedSlider->draw(window);
	}

	if (showExitConfirm) {
		sf::RectangleShape overlay(sf::Vector2f(windowWidth, windowHeight));
		overlay.setFillColor(Theme::withAlpha(sf::Color::Black, 150));
		window.draw(overlay);

		sf::RectangleShape box(sf::Vector2f(400, 200));
		box.setFillColor(Theme::withAlpha(Theme::background(), 245));
		box.setOutlineThickness(2.f);
		box.setOutlineColor(Theme::withAlpha(Theme::surfaceAlt(), 160));
		box.setOrigin(200, 100);
		box.setPosition(windowWidth / 2.f, windowHeight / 2.f);
		window.draw(box);

		sf::Text qText("Are you sure to exit?", font, 20);
		qText.setFillColor(Theme::currentTheme == Theme::ThemeMode::DARK ? Theme::textOnDark() : Theme::textOnLight());

		sf::FloatRect qb = qText.getLocalBounds();
		qText.setOrigin(qb.left + qb.width / 2.f, qb.top + qb.height / 2.f);
		qText.setPosition(windowWidth / 2.f, windowHeight / 2.f - 40.f);
		window.draw(qText);

		for (auto& btn : exitConfirmButtons)
			btn.draw(window);
	}

	window.display();
}
