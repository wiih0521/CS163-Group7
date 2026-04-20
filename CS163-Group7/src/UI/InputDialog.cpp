#include "UI/InputDialog.h"
#include "UI/Theme.h"
#include <algorithm>

InputDialog::InputDialog(
	const std::string& title, const std::vector<Field>& fields, sf::Font& font,
	std::function<void(const std::vector<std::string>&)> onSubmit,
	std::function<void()> onCancel, float windowWidth, float windowHeight,
	sf::Vector2f position)
	: fontRef(font), submitCallback(onSubmit), cancelCallback(onCancel) {
	backgroundOverlay.setSize(sf::Vector2f(windowWidth, windowHeight));
	backgroundOverlay.setFillColor(Theme::withAlpha(sf::Color::Black, position.x >= 0.f ? 0 : 150));

	float boxWidth  = 350.f;
	float boxHeight = 100.f + fields.size() * 60.f;

	box.setSize(sf::Vector2f(boxWidth, boxHeight));
	box.setFillColor(Theme::withAlpha(Theme::background(), 245));
	box.setOutlineThickness(2.f);
	box.setOutlineColor(Theme::withAlpha(Theme::surfaceAlt(), 160));

	float boxX = (windowWidth - boxWidth) / 2.f;
	float boxY = (windowHeight - boxHeight) / 2.f;
	if (position.x >= 0.f && position.y >= 0.f) {
		boxX = position.x;
		boxY = position.y;
	}
	const float margin = 20.f;
	boxX = std::max(margin, std::min(boxX, windowWidth - boxWidth - margin));
	boxY = std::max(margin, std::min(boxY, windowHeight - boxHeight - margin));
	box.setPosition(boxX, boxY);

	titleText.setFont(font);
	titleText.setString(title);
	titleText.setCharacterSize(20);
	titleText.setFillColor(Theme::textOnDark());
	titleText.setPosition(boxX + 20.f, boxY + 20.f);

	float currentY = boxY + 60.f;

	for (const auto& field : fields) {
		sf::Text label;
		label.setFont(font);
		label.setString(field.label);
		label.setCharacterSize(16);
		label.setFillColor(Theme::textOnDark());
		label.setPosition(boxX + 20.f, currentY + 5.f);
		labels.push_back(label);

		TextInput input(sf::Vector2f(180.f, 30.f), sf::Vector2f(boxX + 150.f, currentY), font, field.placeholder);
		textInputs.push_back(input);

		currentY += 50.f;
	}

	buttons.push_back(Button(sf::Vector2f(100.f, 35.f), sf::Vector2f(boxX + 60.f, currentY + 10.f), "OK", font, [this] () {
		std::vector<std::string> results;
		for (const auto& input : textInputs)
			results.push_back(input.getText());
		submitCallback(results);
	}));

	buttons.push_back(Button(sf::Vector2f(100.f, 35.f), sf::Vector2f(boxX + 190.f, currentY + 10.f), "Cancel", font, [this] () {
		cancelCallback();
	}));
}

void InputDialog::draw(sf::RenderWindow& window) {
	window.draw(backgroundOverlay);
	window.draw(box);
	window.draw(titleText);

	for (auto& label : labels)
		window.draw(label);

	for (auto& input : textInputs)
		input.draw(window);

	for (auto& btn : buttons)
		btn.draw(window);
}

bool InputDialog::handleEvent(const sf::Event& event, const sf::RenderWindow& window) {
	for (auto& btn : buttons) {
		if (btn.handleEvent(event, window))
			return true;
	}

	for (auto& input : textInputs) {
		if (input.handleEvent(event, window))
			return true;
	}

	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
		std::vector<std::string> results;
		for (const auto& input : textInputs)
			results.push_back(input.getText());
		submitCallback(results);
		return true;
	}

	if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape) {
		cancelCallback();
		return true;
	}

	if (event.type == sf::Event::MouseButtonPressed ||
		event.type == sf::Event::MouseButtonReleased ||
		event.type == sf::Event::MouseMoved) {
		if (event.type == sf::Event::MouseButtonPressed) {
			sf::Vector2i mousePos = sf::Mouse::getPosition(window);
			if (!box.getGlobalBounds().contains(sf::Vector2f(mousePos)))
				cancelCallback();
		}
		return true;
	}

	return false;
}
