#pragma once
#include <SFML/Graphics.hpp>

class MouseDetector
{
public:
	bool isOn(sf::Sprite item, sf::RenderWindow &window);

	bool isOn(sf::RectangleShape item, sf::RenderWindow& window);
};
	