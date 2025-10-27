#include "MouseDetector.h"

using namespace sf;

bool MouseDetector::isOn(Sprite item, RenderWindow &window)
{
	if (Mouse::getPosition(window).x > (item.getPosition().x - item.getOrigin().x)
		&& Mouse::getPosition(window).x < (item.getPosition().x + item.getOrigin().x)
		&& Mouse::getPosition(window).y >(item.getPosition().y - item.getOrigin().y)
		&& Mouse::getPosition(window).y < (item.getPosition().y + item.getOrigin().y))
	{
		return true;
	}
    return false;
}

bool MouseDetector::isOn(sf::RectangleShape item, sf::RenderWindow& window)
{
	if (Mouse::getPosition(window).x > (item.getPosition().x - item.getOrigin().x)
		&& Mouse::getPosition(window).x < (item.getPosition().x + item.getOrigin().x)
		&& Mouse::getPosition(window).y >(item.getPosition().y - item.getOrigin().y)
		&& Mouse::getPosition(window).y < (item.getPosition().y + item.getOrigin().y))
	{
		return true;
	}

	return false;
}
