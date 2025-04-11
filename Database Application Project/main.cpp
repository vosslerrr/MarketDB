#include <iostream>
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include "MouseDetector.h"

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
	font.openFromFile("arial.ttf");

	Text text(font);
	text.setString("Button");
	text.setCharacterSize(24);
	text.setFillColor(Color::Red);
	text.setOrigin({ text.getGlobalBounds().getCenter().x, text.getGlobalBounds().getCenter().y});
	text.setPosition({640,360});

	RectangleShape button({75,50});
	button.setOrigin({button.getGeometricCenter().x, button.getGeometricCenter().y});
	button.setPosition({ 640,360 });

	RectangleShape textBox({ 75,25 });
	textBox.setOrigin({ textBox.getGeometricCenter().x, textBox.getGeometricCenter().y });
	textBox.setPosition({ 640, 300 });
	textBox.setFillColor(Color::White);

	string input;
	Text inputText(font);
	inputText.setString("Type Here");
	inputText.setCharacterSize(12);
	inputText.setFillColor(Color::Black);
	inputText.setOrigin({ inputText.getGlobalBounds().getCenter().x, inputText.getGlobalBounds().getCenter().y });
	inputText.setPosition({ 635, 300 });

	MouseDetector mouseDetector;
	Clock clock;
	bool clickBox = false;
	const float waitTime = 0.3;

/*-----------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------*/
	
	while (window.isOpen())
	{	
		while (const optional event = window.pollEvent())
		{
			if (event->is<Event::Closed>())
			{
				SQLDisconnect(dbconSQL);
				window.close();
			}

			if (const auto* textEntered = event->getIf<Event::TextEntered>())
			{
				if (textEntered->unicode < 128)
				{	
					if (clickBox == true)
					{
						input += static_cast<char>(textEntered->unicode);
						inputText.setString(input);
					}
				}
			}
		}

		//update this so that it does not accept blank values
		if (mouseDetector.isOn(button, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{			
			if (clock.getElapsedTime().asSeconds() >= waitTime)
			{
				wstring wInput(input.begin(), input.end());
				wstring insertQuery = L"INSERT INTO test_table (test_insert) VALUES ('" + wInput + L"')";


				//always allocate the handle and then free it when using handleSQL (line 119)
				SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
				retSQL = SQLExecDirect(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);
				SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

				if (SQL_SUCCEEDED(retSQL))
				{
					cout << "Insert successful!" << endl;
				}
				else
				{
					cerr << "Insert failed!" << endl;
				}

				input = "";
				inputText.setString(input);

				clock.restart();
			}
		}

		if (mouseDetector.isOn(textBox, window))
		{
			window.setMouseCursor(Cursor::createFromSystem(Cursor::Type::Text).value());

			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				clickBox = true;
				inputText.setString(input);
			}
		}

		else
		{
			window.setMouseCursor(Cursor::createFromSystem(Cursor::Type::Arrow).value());
		}

		if (!mouseDetector.isOn(textBox, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickBox = false;
		}
		
		window.clear();
		window.draw(textBox);
		window.draw(inputText);
		window.draw(button);
		window.draw(text);
		window.display();
	}
}