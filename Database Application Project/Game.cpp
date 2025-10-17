#include "Game.h"

Game::Game() : m_window("Test", sf::Vector2u(1280, 720)) {}

Game::~Game() {}

void Game::Update()
{
	m_window.Update();
}

void Game::Render()
{
	m_window.BeginDraw();
	//m_window.Draw(something);
	m_window.EndDraw();
}

sf::Time Game::GetElapsed()
{
	return m_elapsed;
}

void Game::RestartClock()
{
	m_elapsed = m_clock.restart();
}

Window& Game::GetWindow()
{
	return m_window;
}