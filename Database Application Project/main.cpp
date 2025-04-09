#include <iostream>
#include <SFML/Graphics.hpp>

using namespace sf;
using namespace std;

int main()
{
	RenderWindow window = RenderWindow(VideoMode({ 1280,720 }), "Test");
	window.setFramerateLimit(60);

	Font font;
	
	if (!font.openFromFile("arial.ttf"))
	{
		cout << "invalid location/name";
	}

	Text text(font);

	text.setString("Button");
	text.setCharacterSize(24);
	text.setFillColor(Color::Red);
	text.setOrigin({ text.getGlobalBounds().getCenter().x, text.getGlobalBounds().getCenter().y});
	text.setPosition({640,360});

	RectangleShape button({75,50});
	button.setOrigin({button.getGeometricCenter().x, button.getGeometricCenter().y});
	button.setPosition({ 640,360 });

	while (window.isOpen())
	{
		while (const optional event = window.pollEvent())
		{
			if (event->is < Event::Closed>())
			{
				window.close();
			}
		}

		if (Mouse::getPosition(window).x > (button.getPosition().x - button.getGeometricCenter().x)
			&& Mouse::getPosition(window).x < (button.getPosition().x + button.getGeometricCenter().x)
			&& Mouse::getPosition(window).y >(button.getPosition().y - button.getGeometricCenter().y)
			&& Mouse::getPosition(window).y < (button.getPosition().y + button.getGeometricCenter().y))
		{
			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				cout << "Button clicked" << endl;
			}
		}

		window.clear();
		window.draw(button);
		window.draw(text);
		window.display();
	}


}