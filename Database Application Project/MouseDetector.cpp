#include "MouseDetector.h"

using namespace sf;

bool MouseDetector::isOn(RectangleShape item, RenderWindow &window)
{
	if (Mouse::getPosition(window).x > (item.getPosition().x - item.getGeometricCenter().x)
		&& Mouse::getPosition(window).x < (item.getPosition().x + item.getGeometricCenter().x)
		&& Mouse::getPosition(window).y >(item.getPosition().y - item.getGeometricCenter().y)
		&& Mouse::getPosition(window).y < (item.getPosition().y + item.getGeometricCenter().y))
	{
		return true;
	}
    return false;
}
