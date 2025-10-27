#pragma once
#include <SFML/Graphics.hpp>

class Window
{
public:
	Window();
	Window(const std::string& l_title, const sf::Vector2u& l_size);
	~Window();

	void BeginDraw();
	void Draw(sf::Drawable& l_drawable);
	void EndDraw();
	void Update();
	void ToggleFullscreen();

	bool IsDone() const;
	bool IsFullscreen() const;
	sf::Vector2u GetWindowSize() const;

private:
	void Setup(const std::string& l_title, const sf::Vector2u& l_size);
	void Destroy();
	void Create();

	sf::RenderWindow m_window;
	std::string m_windowTitle;
	sf::Vector2u m_windowSize;
	bool m_isDone;
	bool m_isFullscreen;
};