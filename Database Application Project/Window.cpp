#include "Windows.h"

Windows::Windows()
{
	Setup("Window", sf::Vector2u(1280, 720));
}

Windows::Windows(const std::string& l_title, const sf::Vector2u& l_size)
{
	Setup(l_title, l_size);
}

Windows::~Windows()
{
	Destroy();
}

void Windows::Setup(const std::string& l_title, const sf::Vector2u& l_size)
{
	m_windowTitle = l_title;
	m_windowSize = l_size;
	m_isFullscreen = false;
	m_isDone = false;
	Create();
}

void Windows::Create()
{
	auto state = (m_isFullscreen ? sf::State::Fullscreen : sf::State::Windowed);
	m_window.create(sf::VideoMode({m_windowSize.x, m_windowSize.y}, 32), m_windowTitle, state);
}

void Windows::Destroy()
{
	m_window.close();
}

void Windows::Update()
{
	while (const std::optional event = m_window.pollEvent())
	{
		if (event->is<sf::Event::Closed>())
		{
			m_isDone = true;
		}

		else if (event->is<sf::Event::KeyPressed>() && event->getIf<sf::Event::KeyPressed>()->code == sf::Keyboard::Key::F5)
		{
			ToggleFullscreen();
		}
	}
}

void Windows::ToggleFullscreen()
{
	m_isFullscreen = !m_isFullscreen;
	Destroy();
	Create();
}

void Windows::BeginDraw()
{
	m_window.clear();
}

void Windows::Draw(sf::Drawable& l_drawable)
{
	m_window.draw(l_drawable);
}

void Windows::EndDraw()
{
	m_window.display();
}

bool Windows::IsDone()
{
	return m_isDone;
}

bool Windows::IsFullscreen()
{
	return m_isFullscreen;
}