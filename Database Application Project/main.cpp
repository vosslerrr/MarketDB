#include <iostream>
#include <SFML/Graphics.hpp>

int main()
{
	sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode({ 1280,720 }), "Test");
	window->setFramerateLimit(60);

	while (window->isOpen())
	{
		while (const std::optional event = window->pollEvent())
		{
			if (event->is < sf::Event::Closed>())
			{
				window->close();
			}
		}

		window->clear();
		window->display();
	}


}