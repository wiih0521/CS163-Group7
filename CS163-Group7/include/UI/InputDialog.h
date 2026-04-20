#pragma once
#include "UI/Button.h"
#include "UI/TextInput.h"
#include <SFML/Graphics.hpp>
#include <functional>
#include <string>
#include <vector>

class InputDialog {
public:
  struct Field {
    std::string label;
    std::string placeholder;
  };

  InputDialog(const std::string &title, const std::vector<Field> &fields,
              sf::Font &font,
              std::function<void(const std::vector<std::string> &)> onSubmit,
              std::function<void()> onCancel, float windowWidth,
              float windowHeight,
              sf::Vector2f position = sf::Vector2f(-1.f, -1.f));

  void draw(sf::RenderWindow &window);
  bool handleEvent(const sf::Event &event, const sf::RenderWindow &window);

private:
  sf::RectangleShape backgroundOverlay;
  sf::RectangleShape box;
  sf::Text titleText;
  sf::Font &fontRef;

  std::vector<sf::Text> labels;
  std::vector<TextInput> textInputs;

  std::vector<Button> buttons;
  std::function<void(const std::vector<std::string> &)> submitCallback;
  std::function<void()> cancelCallback;
};
