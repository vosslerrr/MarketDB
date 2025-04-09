#include <iostream>
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>

using namespace sf;
using namespace std;

int main()
{
	SQLHENV envSQL;
	SQLHDBC dbconSQL;
	SQLHSTMT handleSQL;
	SQLRETURN retSQL;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &envSQL);
	SQLSetEnvAttr(envSQL, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, envSQL, &dbconSQL);

	SQLWCHAR connStr[] = L"DRIVER={MySQL ODBC 8.0 ANSI Driver};SERVER=localhost;PORT=3306;DATABASE=test_db;UID=root;PWD=Capacity16!?;";
	retSQL = SQLDriverConnect(dbconSQL, NULL, connStr, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

	if (SQL_SUCCEEDED(retSQL))
	{
		cout << "Connected" << endl;

		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
		SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
	}
	else
	{
		cerr << "Connection failed" << endl;
	}

	SQLFreeHandle(SQL_HANDLE_DBC, dbconSQL);
	SQLFreeHandle(SQL_HANDLE_ENV, envSQL);

/*----------------------------------------------------------------------------------------------------------------------------------------------- 
-------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------*/
	
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
				SQLDisconnect(dbconSQL);
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