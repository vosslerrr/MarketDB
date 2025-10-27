#include "Game.h"
#include <iostream>

int main()
{
	Game game;

	while (!game.GetWindow().IsDone())
	{
		game.Update();
		game.Render();
		game.RestartClock();
	}
}