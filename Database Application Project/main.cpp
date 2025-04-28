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
	text.setString("Submit");
	text.setCharacterSize(24);
	text.setFillColor(Color::Red);
	text.setOrigin({ text.getGlobalBounds().getCenter().x, text.getGlobalBounds().getCenter().y});
	text.setPosition({640,360});

	/*Texture buttonTex;
	buttonTex.loadFromFile("buttonBox.png");
	Sprite submitButton2(buttonTex);
	submitButton2.setOrigin({submitButton2.getGlobalBounds().getCenter().x, submitButton2.getGlobalBounds().getCenter().y});
	submitButton2.setPosition({ 640,420 });*/

	RectangleShape submitButton({75,50});
	submitButton.setOrigin({submitButton.getGeometricCenter().x, submitButton.getGeometricCenter().y});
	submitButton.setPosition({ 640,360 });

	Text item(font);
	item.setString("Item");
	item.setCharacterSize(24);
	item.setFillColor(Color::Black);
	item.setOrigin({item.getGlobalBounds().getCenter().x, item.getGlobalBounds().getCenter().y});
	item.setPosition({ 75,25 });

	Text isle(font);
	isle.setString("Isle");
	isle.setCharacterSize(24);
	isle.setFillColor(Color::Black);
	isle.setOrigin({ isle.getGlobalBounds().getCenter().x, isle.getGlobalBounds().getCenter().y });
	isle.setPosition({ item.getOrigin().x + 100 ,25});

	Text section(font);
	section.setString("Section");
	section.setCharacterSize(24);
	section.setFillColor(Color::Black);
	section.setOrigin({ section.getGlobalBounds().getCenter().x, section.getGlobalBounds().getCenter().y });
	section.setPosition({ 275,25 });

	Text supplier(font);
	supplier.setString("Supplier");
	supplier.setCharacterSize(24);
	supplier.setFillColor(Color::Black);
	supplier.setOrigin({ supplier.getGlobalBounds().getCenter().x, supplier.getGlobalBounds().getCenter().y });
	supplier.setPosition({ 375,25 });

	Text transaction(font);
	transaction.setString("Transaction");
	transaction.setCharacterSize(24);
	transaction.setFillColor(Color::Black);
	transaction.setOrigin({ transaction.getGlobalBounds().getCenter().x, transaction.getGlobalBounds().getCenter().y });
	transaction.setPosition({ 475,25 });

	RectangleShape textBox({ 75,25 });
	textBox.setOrigin({ textBox.getGeometricCenter().x, textBox.getGeometricCenter().y });
	textBox.setPosition({ 640, 300 });
	textBox.setFillColor(Color::White);

	RectangleShape background({ 1280,720 });
	background.setOrigin({ background.getGeometricCenter().x, background.getGeometricCenter().y });
	background.setPosition({ 640,360 });
	Color backgroundColor(100,100,100);
	background.setFillColor(backgroundColor);

	VertexArray backgroundStrip(PrimitiveType::TriangleStrip, 6);
	backgroundStrip[0].position = {0, 720};
	backgroundStrip[1].position = {50, 720};
	backgroundStrip[2].position = {0,0};
	backgroundStrip[3].position = {50, 50};
	backgroundStrip[4].position = { 1280, 0 };
	backgroundStrip[5].position = { 1280,50 };
	backgroundStrip[0].color = Color::Red;
	backgroundStrip[1].color = Color::Red;
	backgroundStrip[2].color = Color::Red;
	backgroundStrip[3].color = Color::Red;
	backgroundStrip[4].color = Color::Red;
	backgroundStrip[5].color = Color::Red;

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

		if (mouseDetector.isOn(submitButton, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{			
			if (clock.getElapsedTime().asSeconds() >= waitTime)
			{
				wstring wInput(input.begin(), input.end());
				wstring insertQuery = L"INSERT INTO item (item_id) VALUES ('" + wInput + L"')";


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
		window.draw(background);
		window.draw(backgroundStrip);
		window.draw(item);
		window.draw(isle);
		window.draw(section);
		window.draw(supplier);
		window.draw(transaction);
		window.draw(textBox);
		window.draw(inputText);
		window.draw(submitButton);
		//window.draw(submitButton2);
		window.draw(text);
		window.display();
	}
}