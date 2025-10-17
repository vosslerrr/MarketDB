#include <windows.h>
#include "Window.h"

int main()
{
	Window test;

	while (!test.IsDone())
	{
		test.Update();
		test.BeginDraw();
		test.EndDraw();
	}

}