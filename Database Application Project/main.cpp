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

	RenderWindow loginwindow = RenderWindow(VideoMode({ 800,600 }), "Login");
	loginwindow.setFramerateLimit(60);

	RectangleShape loginBackground({800,600});
	loginBackground.setOrigin({loginBackground.getGeometricCenter().x, loginBackground.getGeometricCenter().y});
	loginBackground.setPosition({ 400,300 });
	loginBackground.setFillColor(Color::Green);

	Texture loginColumnsTexture;
	loginColumnsTexture.loadFromFile("loginColumns.png");
	Sprite loginColumns(loginColumnsTexture);
	loginColumns.setPosition({220, 260});

	RectangleShape serverBox({ 150,22 });
	serverBox.setOrigin({ serverBox.getGeometricCenter().x,serverBox.getGeometricCenter().y });
	serverBox.setPosition({ 400,300 });

	RectangleShape portBox({ 150,22 });
	portBox.setOrigin({ portBox.getGeometricCenter().x,portBox.getGeometricCenter().y });
	portBox.setPosition({ 400,340 });

	RectangleShape databaseBox({ 150,22 });
	databaseBox.setOrigin({ databaseBox.getGeometricCenter().x,databaseBox.getGeometricCenter().y });
	databaseBox.setPosition({ 400,380 });

	RectangleShape uidBox({ 150,22 });
	uidBox.setOrigin({ uidBox.getGeometricCenter().x,uidBox.getGeometricCenter().y });
	uidBox.setPosition({ 400,420 });

	RectangleShape pwdBox({ 150,22 });
	pwdBox.setOrigin({ pwdBox.getGeometricCenter().x,pwdBox.getGeometricCenter().y });
	pwdBox.setPosition({ 400,460 });

	Texture loginTexture;
	loginTexture.loadFromFile("loginBox.png");

	Sprite loginBox(loginTexture);
	loginBox.setOrigin({28,12});
	loginBox.setPosition({ 400, 500 });

	Font font;
	font.openFromFile("arial.ttf");

	string serverIn;
	Text serverInput(font);
	serverInput.setCharacterSize(20);
	serverInput.setFillColor(Color::Black);
	serverInput.setOrigin({ serverInput.getGlobalBounds().getCenter().x, serverInput.getGlobalBounds().getCenter().y });
	serverInput.setPosition({ 330,285 });

	string portIn;
	Text portInput(font);
	portInput.setCharacterSize(20);
	portInput.setFillColor(Color::Black);
	portInput.setOrigin({ portInput.getGlobalBounds().getCenter().x, portInput.getGlobalBounds().getCenter().y });
	portInput.setPosition({ 330,325 });

	string databaseIn;
	Text databaseInput(font);
	databaseInput.setCharacterSize(20);
	databaseInput.setFillColor(Color::Black);
	databaseInput.setOrigin({ databaseInput.getGlobalBounds().getCenter().x, databaseInput.getGlobalBounds().getCenter().y });
	databaseInput.setPosition({ 330,365 });

	string uidIn;
	Text uidInput(font);
	uidInput.setCharacterSize(20);
	uidInput.setFillColor(Color::Black);
	uidInput.setOrigin({ uidInput.getGlobalBounds().getCenter().x, uidInput.getGlobalBounds().getCenter().y });
	uidInput.setPosition({ 330,405 });

	string pwdIn;
	Text pwdInput(font);
	pwdInput.setCharacterSize(20);
	pwdInput.setFillColor(Color::Black);
	pwdInput.setOrigin({ pwdInput.getGlobalBounds().getCenter().x, pwdInput.getGlobalBounds().getCenter().y });
	pwdInput.setPosition({ 330,445 });

	bool clickServer = false;
	bool clickPort = false;
	bool clickDatabase = false;
	bool clickUid = false;
	bool clickPwd = false;

	MouseDetector winLogDetector;
	Clock loginClock;

	while (loginwindow.isOpen())
	{
		while (const optional event = loginwindow.pollEvent())
		{
			if (event->is<Event::Closed>())
			{
				loginwindow.close();
				return 0;
			}

			if (const auto* textEntered = event->getIf<Event::TextEntered>())
			{
				if (textEntered->unicode > 32 && textEntered->unicode < 127)
				{
					if (clickServer)
					{
						serverIn += static_cast<char>(textEntered->unicode);
						serverInput.setString(serverIn);
					}

					if (clickPort)
					{
						portIn += static_cast<char>(textEntered->unicode);
						portInput.setString(portIn);
					}

					if (clickDatabase)
					{
						databaseIn += static_cast<char>(textEntered->unicode);
						databaseInput.setString(databaseIn);
					}

					if (clickUid)
					{
						uidIn += static_cast<char>(textEntered->unicode);
						uidInput.setString(uidIn);
					}

					if (clickPwd)
					{
						pwdIn += static_cast<char>(textEntered->unicode);
						pwdInput.setString(pwdIn);
					}
				}

				if (textEntered->unicode == 8)
				{
					if (clickServer)
					{
						if (serverIn != "")
						{
							serverIn.pop_back();
							serverInput.setString(serverIn);
						}
					}

					if (clickPort)
					{
						if (portIn != "")
						{
							portIn.pop_back();
							portInput.setString(portIn);
						}
					}

					if (clickDatabase)
					{
						if (databaseIn != "")
						{
							databaseIn.pop_back();
							databaseInput.setString(databaseIn);
						}
					}

					if (clickUid)
					{
						if (uidIn != "")
						{
							uidIn.pop_back();
							uidInput.setString(uidIn);
						}
					}

					if (clickPwd)
					{
						if (pwdIn != "")
						{
							pwdIn.pop_back();
							pwdInput.setString(pwdIn);
						}
					}
				}
			}
		}

		if (winLogDetector.isOn(loginBox, loginwindow) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			if (loginClock.getElapsedTime().asSeconds() >= 0.3)
			{
				wstring wServerInput(serverIn.begin(), serverIn.end());
				wstring wPortInput(portIn.begin(), portIn.end());
				wstring wDatabaseInput(databaseIn.begin(), databaseIn.end());
				wstring wUidInput(uidIn.begin(), uidIn.end());
				wstring wPwdInput(pwdIn.begin(), pwdIn.end());
				wstring connStr = L"DRIVER={MySQL ODBC 8.0 ANSI Driver};SERVER=" + wServerInput + L";PORT=" + wPortInput + L";DATABASE=" + 
								  wDatabaseInput + L";UID=" + wUidInput + L";PWD=" + wPwdInput + L";";

				retSQL = SQLDriverConnect(dbconSQL, NULL, (SQLWCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

				if (SQL_SUCCEEDED(retSQL))
				{
					cout << "Connected" << endl;

					SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
					SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
					SQLFreeHandle(SQL_HANDLE_DBC, dbconSQL);
					SQLFreeHandle(SQL_HANDLE_ENV, envSQL);

					loginwindow.close();
				}

				else
				{
					cerr << "Connection failed" << endl;

					serverIn = "";
					serverInput.setString(serverIn);
					portIn = "";
					portInput.setString(portIn);
					databaseIn = "";
					databaseInput.setString(databaseIn);
					uidIn = "";
					uidInput.setString(uidIn);
					pwdIn = "";
					pwdInput.setString(pwdIn);
				}

				loginClock.restart();
			}
		}

		if (winLogDetector.isOn(serverBox, loginwindow) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickServer = true;
			clickPort = false;
			clickDatabase = false;
			clickUid = false;
			clickPwd = false;

			serverIn = "localhost";
			serverInput.setString(serverIn);
		}

		if (winLogDetector.isOn(portBox, loginwindow) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickServer = false;
			clickPort = true;
			clickDatabase = false;
			clickUid = false;
			clickPwd = false;

			portIn = "3306";
			portInput.setString(portIn);
		}

		if (winLogDetector.isOn(databaseBox, loginwindow) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickServer = false;
			clickPort = false;
			clickDatabase = true;
			clickUid = false;
			clickPwd = false;

			databaseIn = "test_db";
			databaseInput.setString(databaseIn);
		}

		if (winLogDetector.isOn(uidBox, loginwindow) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickServer = false;
			clickPort = false;
			clickDatabase = false;
			clickUid = true;
			clickPwd = false;

			uidIn = "root";
			uidInput.setString(uidIn);
		}

		if (winLogDetector.isOn(pwdBox, loginwindow) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickServer = false;
			clickPort = false;
			clickDatabase = false;
			clickUid = false;
			clickPwd = true;

			pwdIn = "Capacity16!?";
			pwdInput.setString(pwdIn);
		}

		loginwindow.clear();
		loginwindow.draw(loginBackground);
		loginwindow.draw(loginColumns);
		loginwindow.draw(serverBox);
		loginwindow.draw(serverInput);
		loginwindow.draw(portBox);
		loginwindow.draw(portInput);
		loginwindow.draw(databaseBox);
		loginwindow.draw(databaseInput);
		loginwindow.draw(uidBox);
		loginwindow.draw(uidInput);
		loginwindow.draw(pwdBox);
		loginwindow.draw(pwdInput);
		loginwindow.draw(loginBox);
		loginwindow.display();
	}
	
	

/*----------------------------------------------------------------------------------------------------------------------------------------------- 
-------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------*/
	
	RenderWindow window = RenderWindow(VideoMode({ 1280,720 }), "Test");
	window.setFramerateLimit(60);

	Texture submitButton;
	submitButton.loadFromFile("buttonBox.png");
	Sprite submitButtonSprite(submitButton);
	submitButtonSprite.setOrigin({ 24,8 });
	submitButtonSprite.setPosition({ 210,500 });

	Texture headers;
	headers.loadFromFile("headers.png");
	Sprite headersSprite(headers);
	headersSprite.setPosition({ 75,15 });

	RectangleShape itemHeaderBox({57,23});
	itemHeaderBox.setPosition({79,17});
	itemHeaderBox.setFillColor(Color::Blue);

	RectangleShape isleHeaderBox({ 45,23 });
	isleHeaderBox.setPosition({ 221,17 });
	isleHeaderBox.setFillColor(Color::Blue);

	RectangleShape sectionHeaderBox({ 104,23 });
	sectionHeaderBox.setPosition({ 349,17 });
	sectionHeaderBox.setFillColor(Color::Blue);

	RectangleShape supplierHeaderBox({ 117,23 });
	supplierHeaderBox.setPosition({ 535,17 });
	supplierHeaderBox.setFillColor(Color::Blue);

	RectangleShape transactionHeaderBox({ 164,23 });
	transactionHeaderBox.setPosition({ 731,17 });
	transactionHeaderBox.setFillColor(Color::Blue);

	Texture itemColumns;
	itemColumns.loadFromFile("itemColumns.png");
	Sprite itemColumnsSprite(itemColumns);
	itemColumnsSprite.setPosition({ 75,100 });

	string t1In;
	Text t1Input(font);
	t1Input.setCharacterSize(20);
	t1Input.setFillColor(Color::Black);
	t1Input.setOrigin({ t1Input.getGlobalBounds().getCenter().x, t1Input.getGlobalBounds().getCenter().y });
	t1Input.setPosition({ 206,95 });

	RectangleShape textBox1({ 150,22 });
	textBox1.setOrigin({ textBox1.getGeometricCenter().x, textBox1.getGeometricCenter().y });
	textBox1.setPosition({ 280, 110 });
	textBox1.setFillColor(Color::White);

	string t2In;
	Text t2Input(font);
	t2Input.setCharacterSize(20);
	t2Input.setFillColor(Color::Black);
	t2Input.setOrigin({ t2Input.getGlobalBounds().getCenter().x, t2Input.getGlobalBounds().getCenter().y });
	t2Input.setPosition({ 206,143 });

	RectangleShape textBox2({ 150,22 });
	textBox2.setOrigin({ textBox2.getGeometricCenter().x, textBox2.getGeometricCenter().y });
	textBox2.setPosition({ 280, 158 });
	textBox2.setFillColor(Color::White);

	string t3In;
	Text t3Input(font);
	t3Input.setCharacterSize(20);
	t3Input.setFillColor(Color::Black);
	t3Input.setOrigin({ t3Input.getGlobalBounds().getCenter().x, t3Input.getGlobalBounds().getCenter().y });
	t3Input.setPosition({ 206,191 });

	RectangleShape textBox3({ 150,22 });
	textBox3.setOrigin({ textBox3.getGeometricCenter().x, textBox3.getGeometricCenter().y });
	textBox3.setPosition({ 280, 206 });
	textBox3.setFillColor(Color::White);

	string t4In;
	Text t4Input(font);
	t4Input.setCharacterSize(20);
	t4Input.setFillColor(Color::Black);
	t4Input.setOrigin({ t4Input.getGlobalBounds().getCenter().x, t4Input.getGlobalBounds().getCenter().y });
	t4Input.setPosition({ 206,239 });

	RectangleShape textBox4({ 150,22 });
	textBox4.setOrigin({ textBox4.getGeometricCenter().x, textBox4.getGeometricCenter().y });
	textBox4.setPosition({ 280, 254 });
	textBox4.setFillColor(Color::White);

	string t5In;
	Text t5Input(font);
	t5Input.setCharacterSize(20);
	t5Input.setFillColor(Color::Black);
	t5Input.setOrigin({ t5Input.getGlobalBounds().getCenter().x, t5Input.getGlobalBounds().getCenter().y });
	t5Input.setPosition({ 206,287 });

	RectangleShape textBox5({ 150,22 });
	textBox5.setOrigin({ textBox5.getGeometricCenter().x, textBox5.getGeometricCenter().y });
	textBox5.setPosition({ 280, 302 });
	textBox5.setFillColor(Color::White);

	string t6In;
	Text t6Input(font);
	t6Input.setCharacterSize(20);
	t6Input.setFillColor(Color::Black);
	t6Input.setOrigin({ t6Input.getGlobalBounds().getCenter().x, t6Input.getGlobalBounds().getCenter().y });
	t6Input.setPosition({ 206,335 });

	RectangleShape textBox6({ 150,22 });
	textBox6.setOrigin({ textBox6.getGeometricCenter().x, textBox6.getGeometricCenter().y });
	textBox6.setPosition({ 280, 350 });
	textBox6.setFillColor(Color::White);

	RectangleShape checkBox({ 20,22 });
	checkBox.setPosition({ 210, 386 });
	checkBox.setFillColor(Color::White);

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

	MouseDetector mouseDetector;
	Clock clock;
	bool clickT1 = false;
	bool clickT2 = false;
	bool clickT3 = false;
	bool clickT4 = false;
	bool clickT5 = false;
	bool clickT6 = false;

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
				if (textEntered->unicode > 32 && textEntered->unicode < 127)
				{	
					if (clickT1)
					{
						t1In += static_cast<char>(textEntered->unicode);
						t1Input.setString(t1In);
					}

					if (clickT2)
					{
						t2In += static_cast<char>(textEntered->unicode);
						t2Input.setString(t2In);
					}

					if (clickT3)
					{
						t3In += static_cast<char>(textEntered->unicode);
						t3Input.setString(t3In);
					}

					if (clickT4)
					{
						t4In += static_cast<char>(textEntered->unicode);
						t4Input.setString(t4In);
					}

					if (clickT5)
					{
						t5In += static_cast<char>(textEntered->unicode);
						t5Input.setString(t5In);
					}

					if (clickT6)
					{
						t6In += static_cast<char>(textEntered->unicode);
						t6Input.setString(t6In);
					}
				}
			}
		}

		if (mouseDetector.isOn(textBox1, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickT1 = true;
			clickT2 = false;
			clickT3 = false;
			clickT4 = false;
			clickT5 = false;
			clickT6 = false;

			t1Input.setString(t1In);
		}

		if (mouseDetector.isOn(textBox2, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickT1 = false;
			clickT2 = true;
			clickT3 = false;
			clickT4 = false;
			clickT5 = false;
			clickT6 = false;

			t2Input.setString(t2In);
		}

		if (mouseDetector.isOn(textBox3, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickT1 = false;
			clickT2 = false;
			clickT3 = true;
			clickT4 = false;
			clickT5 = false;
			clickT6 = false;

			t3Input.setString(t3In);
		}

		if (mouseDetector.isOn(textBox4, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickT1 = false;
			clickT2 = false;
			clickT3 = false;
			clickT4 = true;
			clickT5 = false;
			clickT6 = false;

			t4Input.setString(t4In);
		}

		if (mouseDetector.isOn(textBox5, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickT1 = false;
			clickT2 = false;
			clickT3 = false;
			clickT4 = false;
			clickT5 = true;
			clickT6 = false;

			t5Input.setString(t5In);
		}

		if (mouseDetector.isOn(textBox6, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{
			clickT1 = false;
			clickT2 = false;
			clickT3 = false;
			clickT4 = false;
			clickT5 = false;
			clickT6 = true;

			t6Input.setString(t6In);
		}

		if (mouseDetector.isOn(submitButtonSprite, window) && Mouse::isButtonPressed(Mouse::Button::Left))
		{			
			if (clock.getElapsedTime().asSeconds() >= 0.3)
			{ 
				SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

				wstring item_id(t1In.begin(), t1In.end());
				wstring item_name(t2In.begin(), t2In.end());
				int isle_no = stoi(t3In);
				wstring section_id(t4In.begin(), t4In.end());
				float item_price = stod(t5In);
				int no_of_items = stoi(t6In);
				wstring insertQuery = L"INSERT INTO item (item_id, item_name, isle_no, section_id, item_price, no_of_items) VALUES (?,?,?,?,?,?)";
				
				SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

				SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
				SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_name.c_str(), 0, NULL);
				SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &isle_no, 0, NULL);
				SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
				SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &item_price, 0, NULL);
				SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_items, 0, NULL);

				retSQL = SQLExecute(handleSQL);

				if (SQL_SUCCEEDED(retSQL))
				{
					cout << "Insert successful!" << endl;
				}
				else
				{
					cerr << "Insert failed!" << endl;

					SQLWCHAR sqlState[6], message[256];
					SQLINTEGER nativeError;
					SQLSMALLINT textLength;
					if (SQLGetDiagRec(SQL_HANDLE_STMT, handleSQL, 1, sqlState, &nativeError, message, sizeof(message) / sizeof(SQLWCHAR), &textLength) == SQL_SUCCESS)
					{
						wcerr << L"ODBC Error: " << message << L" (SQLSTATE: " << sqlState << L")" << endl;
					}
				}

				SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

				clock.restart();
			}
		}
		
		window.clear();
		window.draw(background);
		window.draw(backgroundStrip);
		window.draw(headersSprite);
		window.draw(textBox1);
		window.draw(t1Input);
		window.draw(textBox2);
		window.draw(t2Input);
		window.draw(textBox3);
		window.draw(t3Input);
		window.draw(textBox4);
		window.draw(t4Input);
		window.draw(textBox5);
		window.draw(t5Input);
		window.draw(textBox6);
		window.draw(t6Input);
		window.draw(checkBox);
		window.draw(submitButtonSprite);
		window.draw(itemColumnsSprite);
		window.display();
	}
}