#include <imgui.h>
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include "MouseDetector.h"
#include <imgui-SFML.h>

using namespace sf;
using namespace std;

bool IsDriverInstalled() 
{
	HKEY hKey;
	const wchar_t* subKey = L"SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers";

	if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, subKey, 0, KEY_READ, &hKey) != ERROR_SUCCESS) {return false;}

	wchar_t valueName[256];
	DWORD valueNameSize = sizeof(valueName) / sizeof(wchar_t);
	BYTE data[256];
	DWORD dataSize = sizeof(data);
	DWORD type;
	DWORD index = 0;

	bool driverFound = false;

	while (RegEnumValueW(hKey, index++, valueName, &valueNameSize, nullptr, &type, data, &dataSize) == ERROR_SUCCESS) {
		if (type == REG_SZ) {
			if (wcscmp(valueName, L"MySQL ODBC 8.0 ANSI Driver") == 0) {
				driverFound = true;
				break;
			}
		}

		valueNameSize = sizeof(valueName) / sizeof(wchar_t);
		dataSize = sizeof(data);
	}

	RegCloseKey(hKey);
	return driverFound;
}

int main()
{
	if (!IsDriverInstalled()) 
	{
		int result = MessageBoxW(
			nullptr,
			L"The MySQL ODBC 8.0 ANSI Driver is not installed.\n"
			L"This application requires this driver to function.\n"
			L"Once download and installation is complete, please restart the application.\n\n"
			L"Click Yes to download, or No to cancel.",
			L"Missing ODBC Driver",
			MB_YESNO | MB_ICONWARNING
		);

		if (result == IDYES) 
		{
			ShellExecuteW(
				nullptr,
				L"open",
				L"https://dev.mysql.com/get/Downloads/Connector-ODBC/8.0/mysql-connector-odbc-8.0.42-win32.msi",
				nullptr,
				nullptr,
				SW_SHOWNORMAL
			);
		}
		else 
		{
			MessageBoxW(nullptr, L"Installation canceled by user.", L"Canceled", MB_OK | MB_ICONINFORMATION);
		}

		return 3;
	}

	SQLHENV envSQL;
	SQLHDBC dbconSQL;
	SQLHSTMT handleSQL;
	SQLRETURN retSQL;

	SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &envSQL);
	SQLSetEnvAttr(envSQL, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);
	SQLAllocHandle(SQL_HANDLE_DBC, envSQL, &dbconSQL);

	RenderWindow loginwindow = RenderWindow(VideoMode({ 800,600 }), "Login", Style::Close);
	loginwindow.setFramerateLimit(60);

	ImGui::SFML::Init(loginwindow); 
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigInputTextCursorBlink = false;
	io.KeyRepeatDelay = 100.f;
	io.KeyRepeatRate = 15.f;

	Texture loginBackgroundTexture;
	loginBackgroundTexture.loadFromFile("loginColumns.png");
	Sprite loginBackground(loginBackgroundTexture);

	Texture loginTexture;
	loginTexture.loadFromFile("loginBox.png");
	Sprite loginBox(loginTexture);
	loginBox.setOrigin({28,12});
	loginBox.setPosition({ 400, 500 });

	Font font;
	font.openFromFile("arial.ttf");

	ImFont* guiFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("arial.ttf", 20.f);
	ImGui::SFML::UpdateFontTexture();

	bool itemTable = false;
	bool aisleTable = false;
	bool sectionTable = false;
	bool supplierTable = false;
	bool transactionTable = false;

	MouseDetector winLogDetector;
	Clock loginClock;

	char serverIn[128] = "localhost";
	char portIn[128] = "3306";
	char databaseIn[128] = "test_db";
	char uidIn[128] = "root";
	char pwdIn[128] = "Capacity16!?";

	const auto arrowCursor = Cursor::createFromSystem(Cursor::Type::Arrow).value();
	const auto handCursor = Cursor::createFromSystem(Cursor::Type::Hand).value();
	const auto textCursor = Cursor::createFromSystem(Cursor::Type::Text).value();

	while (loginwindow.isOpen())
	{
		while (const optional event = loginwindow.pollEvent())
		{
			ImGui::SFML::ProcessEvent(loginwindow, *event);

			if (event->is<Event::Closed>())
			{
				loginwindow.close();
				return 0;
			}
		}

		ImGui::SFML::Update(loginwindow, loginClock.getElapsedTime());

		//server box
		{
			ImGui::SetNextWindowPos(ImVec2(317, 279));//x-offset:8	y-offset:9
			ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
			ImGui::PushFont(guiFont);
			ImGui::Begin("##ServerInputWindow", nullptr, ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoBackground
				| ImGuiWindowFlags_NoTitleBar);
			ImGui::InputText("##ServerInput", serverIn, sizeof(serverIn));
			ImGui::PopStyleColor(2);
			ImGui::PopFont();
			ImGui::End();
		}

		//port box
		{
			ImGui::SetNextWindowPos(ImVec2(317, 319));//x-offset:8	y-offset:9
			ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
			ImGui::PushFont(guiFont);
			ImGui::Begin("##PortInputWindow", nullptr, ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoBackground
				| ImGuiWindowFlags_NoTitleBar);
			ImGui::InputText("##PortInput", portIn, sizeof(portIn));
			ImGui::PopStyleColor(2);
			ImGui::PopFont();
			ImGui::End();
		}

		//database box
		{
			ImGui::SetNextWindowPos(ImVec2(317, 359));//x-offset:8	y-offset:9
			ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
			ImGui::PushFont(guiFont);
			ImGui::Begin("##DatabaseInputWindow", nullptr, ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoBackground
				| ImGuiWindowFlags_NoTitleBar);
			ImGui::InputText("##DatabaseInput", databaseIn, sizeof(databaseIn));
			ImGui::PopStyleColor(2);
			ImGui::PopFont();
			ImGui::End();
		}

		//uid box
		{
			ImGui::SetNextWindowPos(ImVec2(317, 399));//x-offset:8	y-offset:9
			ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
			ImGui::PushFont(guiFont);
			ImGui::Begin("##UIDInputWindow", nullptr, ImGuiWindowFlags_NoResize
				| ImGuiWindowFlags_NoMove
				| ImGuiWindowFlags_NoCollapse
				| ImGuiWindowFlags_NoBackground
				| ImGuiWindowFlags_NoTitleBar);
			ImGui::InputText("##UIDInput", uidIn, sizeof(uidIn));
			ImGui::PopStyleColor(2);
			ImGui::PopFont();
			ImGui::End();
		}

		//pwd box
		{
			ImGui::SetNextWindowPos(ImVec2(317, 439));//x-offset:8	y-offset:9
			ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
			ImGui::PushFont(guiFont);
			ImGui::Begin("##PWDInputWindow", nullptr, ImGuiWindowFlags_NoResize
													| ImGuiWindowFlags_NoMove
													| ImGuiWindowFlags_NoCollapse
													| ImGuiWindowFlags_NoBackground
													| ImGuiWindowFlags_NoTitleBar);
			ImGui::InputText("##PWDInput", pwdIn, sizeof(pwdIn));
			ImGui::PopStyleColor(2);
			ImGui::PopFont();
			ImGui::End();
		}

		if (winLogDetector.isOn(loginBox, loginwindow))
		{
			loginwindow.setMouseCursor(handCursor);

			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				if (loginClock.getElapsedTime().asSeconds() >= 0.3)
				{
					string serverInStr(serverIn);
					wstring wServerInput(serverInStr.begin(), serverInStr.end());
					string portInStr(portIn);
					wstring wPortInput(portInStr.begin(), portInStr.end());
					string databaseInStr(databaseIn);
					wstring wDatabaseInput(databaseInStr.begin(), databaseInStr.end());
					string uidInStr(uidIn);
					wstring wUidInput(uidInStr.begin(), uidInStr.end());
					string pwdInStr(pwdIn);
					wstring wPwdInput(pwdInStr.begin(), pwdInStr.end());
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

						serverIn[0] = '\0';
						serverInStr = "";
						portIn[0] = '\0';
						portInStr = "";
						databaseIn[0] = '\0';
						databaseInStr = "";
						uidIn[0] = '\0';
						uidInStr = "";
						pwdIn[0] = '\0';
						pwdInStr = "";
					}

					loginClock.restart();
				}
			}
		}

		loginwindow.clear();
		loginwindow.draw(loginBackground);
		loginwindow.draw(loginBox);
		ImGui::SFML::Render(loginwindow);
		loginwindow.display();
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'item'", SQL_NTS);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			itemTable = true;
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (!itemTable)
	{
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

		const wchar_t* createQuery =
			L"CREATE TABLE item ("
			L"item_id VARCHAR(45) PRIMARY KEY NOT NULL,"
			L"item_name VARCHAR(45) NOT NULL,"
			L"aisle_no INT NOT NULL,"
			L"section_id VARCHAR(45) NOT NULL,"
			L"item_price FLOAT NOT NULL,"
			L"no_of_items INT NOT NULL"
			L")";

		retSQL = SQLExecDirect(handleSQL, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'aisle'", SQL_NTS);
	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			aisleTable = true;
		}
	}

	if (!aisleTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE aisle ("
			L"aisle_no INT PRIMARY KEY NOT NULL,"
			L"no_of_sections INT NOT NULL"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'section'", SQL_NTS);
	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			sectionTable = true;
		}
	}

	if (!sectionTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE section ("
			L"section_id VARCHAR(45) PRIMARY KEY NOT NULL,"
			L"section_name VARCHAR(45) NOT NULL,"
			L"aisle_no INT NOT NULL"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'supplier'", SQL_NTS);
	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			supplierTable = true;
		}
	}

	if (!supplierTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE supplier ("
			L"supplier_id VARCHAR(45) PRIMARY KEY NOT NULL,"
			L"item_id VARCHAR(45) NOT NULL,"
			L"item_cost FLOAT NOT NULL,"
			L"supplier_name VARCHAR(45) NOT NULL"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'transaction'", SQL_NTS);
	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			transactionTable = true;
		}
	}

	if (!transactionTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE transaction ("
			L"transaction_id VARCHAR(45) PRIMARY KEY NOT NULL,"
			L"item_id VARCHAR(45) NOT NULL,"
			L"item_price FLOAT NOT NULL,"
			L"tax_amount FLOAT NOT NULL,"
			L"transaction_total FLOAT NOT NULL,"
			L"transaction_date VARCHAR(45) NOT NULL"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
	}

/*----------------------------------------------------------------------------------------------------------------------------------------------- 
-------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------*/
	
	RenderWindow window = RenderWindow(VideoMode({ 1280,720 }), "Test", Style::Close);
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGuiIO& io2 = ImGui::GetIO();
	io2.ConfigInputTextCursorBlink = false;
	io2.KeyRepeatDelay = 100.f;
	io2.KeyRepeatRate = 15.f;

	Texture backgroundTexture;
	backgroundTexture.loadFromFile("background.png");
	Texture itemBackgroundTexture;
	itemBackgroundTexture.loadFromFile("itemBackground.png");
	Texture aisleBackgroundTexture;
	aisleBackgroundTexture.loadFromFile("aisleBackground.png");
	Texture sectionBackgroundTexture;
	sectionBackgroundTexture.loadFromFile("sectionBackground.png");
	Texture supplierBackgroundTexture;
	supplierBackgroundTexture.loadFromFile("supplierBackground.png");
	Texture transactionBackgroundTexture;
	transactionBackgroundTexture.loadFromFile("transactionBackground.png");

	Sprite background(backgroundTexture);
	background.setOrigin({640,360});
	background.setPosition({ 640,360 });

	RectangleShape itemHeaderBox({57,23});
	itemHeaderBox.setOrigin({ itemHeaderBox.getGeometricCenter().x, itemHeaderBox.getGeometricCenter().y });
	itemHeaderBox.setPosition({112,28});
	itemHeaderBox.setOutlineThickness(2);
	itemHeaderBox.setOutlineColor(Color::Blue);
	itemHeaderBox.setFillColor(Color::Transparent);

	RectangleShape aisleHeaderBox({ 69,23 });
	aisleHeaderBox.setOrigin({ aisleHeaderBox.getGeometricCenter().x, aisleHeaderBox.getGeometricCenter().y });
	aisleHeaderBox.setPosition({ 237,28 });
	aisleHeaderBox.setOutlineThickness(2);
	aisleHeaderBox.setOutlineColor(Color::Blue);
	aisleHeaderBox.setFillColor(Color::Transparent);

	RectangleShape sectionHeaderBox({ 104,23 });
	sectionHeaderBox.setOrigin({ sectionHeaderBox.getGeometricCenter().x, sectionHeaderBox.getGeometricCenter().y });
	sectionHeaderBox.setPosition({ 388,28 });
	sectionHeaderBox.setOutlineThickness(2);
	sectionHeaderBox.setOutlineColor(Color::Blue);
	sectionHeaderBox.setFillColor(Color::Transparent);

	RectangleShape supplierHeaderBox({ 116,23 });
	supplierHeaderBox.setOrigin({ supplierHeaderBox.getGeometricCenter().x, supplierHeaderBox.getGeometricCenter().y });
	supplierHeaderBox.setPosition({ 563,28 });
	supplierHeaderBox.setOutlineThickness(2);
	supplierHeaderBox.setOutlineColor(Color::Blue);
	supplierHeaderBox.setFillColor(Color::Transparent);

	RectangleShape transactionHeaderBox({ 165,23 });
	transactionHeaderBox.setOrigin({ transactionHeaderBox.getGeometricCenter().x, transactionHeaderBox.getGeometricCenter().y });
	transactionHeaderBox.setPosition({ 764,28 });
	transactionHeaderBox.setOutlineThickness(2);
	transactionHeaderBox.setOutlineColor(Color::Blue);
	transactionHeaderBox.setFillColor(Color::Transparent);

	RectangleShape textBox1({ 150,22 });
	textBox1.setOrigin({ textBox1.getGeometricCenter().x, textBox1.getGeometricCenter().y });
	textBox1.setPosition({ 0, 0 });
	textBox1.setFillColor(Color::Transparent);

	RectangleShape textBox2({ 150,22 });
	textBox2.setOrigin({ textBox2.getGeometricCenter().x, textBox2.getGeometricCenter().y });
	textBox2.setPosition({ 0,0 });
	textBox2.setFillColor(Color::Transparent);

	RectangleShape textBox3({ 150,22 });
	textBox3.setOrigin({ textBox3.getGeometricCenter().x, textBox3.getGeometricCenter().y });
	textBox3.setPosition({ 0,0 });
	textBox3.setFillColor(Color::Transparent);

	RectangleShape textBox4({ 150,22 });
	textBox4.setOrigin({ textBox4.getGeometricCenter().x, textBox4.getGeometricCenter().y });
	textBox4.setPosition({ 0,0 });
	textBox4.setFillColor(Color::Transparent);

	RectangleShape textBox5({ 150,22 });
	textBox5.setOrigin({ textBox5.getGeometricCenter().x, textBox5.getGeometricCenter().y });
	textBox5.setPosition({ 0,0 });
	textBox5.setFillColor(Color::Transparent);

	RectangleShape textBox6({ 150,22 });
	textBox6.setOrigin({ textBox6.getGeometricCenter().x, textBox6.getGeometricCenter().y });
	textBox6.setPosition({ 0,0 });
	textBox6.setFillColor(Color::Transparent);

	Texture submitButtonTexture;
	submitButtonTexture.loadFromFile("buttonBox.png");
	Sprite submitButton(submitButtonTexture);
	submitButton.setOrigin({ 24,8 });
	submitButton.setPosition({ 0,0 });
	submitButton.setColor(Color::Transparent);

	Texture modifyButtonTexture;
	modifyButtonTexture.loadFromFile("modifyBox.png");
	Sprite modifyButton(modifyButtonTexture);
	modifyButton.setOrigin({ 24,8 });
	modifyButton.setPosition({0,0});
	modifyButton.setColor(Color::Transparent);

	MouseDetector mouseDetector;
	Clock clock;
	bool notNull = true;
	bool valNums = true;
	bool clickItem = false;
	bool clickAisle = false;
	bool clickSection = false;
	bool clickSupplier = false;
	bool clickTransaction = false;

	struct Item {
		string item_id1;
		string item_name1;
		int aisle_no1;
		string section_id1;
		float item_price1;
		int no_of_items1;
	};

	string original_item_id1;

	vector<Item> items;

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

	SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT item_id, item_name, aisle_no, section_id, item_price, no_of_items FROM item", SQL_NTS);

	char item_id1[46], item_name1[46], section_id1[46];
	int aisle_no1, no_of_items1;
	float item_price1;

	SQLBindCol(handleSQL, 1, SQL_C_CHAR, item_id1, sizeof(item_id1), nullptr);
	SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_name1, sizeof(item_name1), nullptr);
	SQLBindCol(handleSQL, 3, SQL_C_SLONG, &aisle_no1, 0, nullptr);
	SQLBindCol(handleSQL, 4, SQL_C_CHAR, section_id1, sizeof(section_id1), nullptr);
	SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &item_price1, 0, nullptr);
	SQLBindCol(handleSQL, 6, SQL_C_SLONG, &no_of_items1, 0, nullptr);

	items.clear();

	while (SQLFetch(handleSQL) == SQL_SUCCESS)
	{
		items.push_back({
			item_id1,
			item_name1,
			aisle_no1,
			section_id1,
			item_price1,
			no_of_items1
			});
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


	struct Aisle {
		int aisle_no2;
		int no_of_sections2;
	};

	int original_aisle_no2;

	vector<Aisle> aisles;

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

	SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT aisle_no, no_of_sections FROM aisle", SQL_NTS);

	int aisle_no2, no_of_sections2;

	SQLBindCol(handleSQL, 1, SQL_C_SLONG, &aisle_no2, 0, nullptr);
	SQLBindCol(handleSQL, 2, SQL_C_SLONG, &no_of_sections2, 0, nullptr);

	aisles.clear();

	while (SQLFetch(handleSQL) == SQL_SUCCESS)
	{
		aisles.push_back({
			aisle_no2,
			no_of_sections2
			});
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


	struct Section {
		string section_id3;
		string section_name3;
		int aisle_no3;
	};

	string original_section_id3;

	vector<Section> sections;

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

	SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT section_id, section_name, aisle_no FROM section", SQL_NTS);

	char section_id3[46], section_name3[46];
	int aisle_no3;

	SQLBindCol(handleSQL, 1, SQL_C_CHAR, section_id3, sizeof(section_id3), nullptr);
	SQLBindCol(handleSQL, 2, SQL_C_CHAR, section_name3, sizeof(section_name3), nullptr);
	SQLBindCol(handleSQL, 3, SQL_C_SLONG, &aisle_no3, 0, nullptr);

	sections.clear();

	while (SQLFetch(handleSQL) == SQL_SUCCESS)
	{
		sections.push_back({
			section_id3,
			section_name3,
			aisle_no3
			});
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


	struct Supplier {
		string supplier_id4;
		string item_id4;
		float item_cost4;
		string supplier_name4;
	};

	string original_supplier_id4;

	vector<Supplier> suppliers;

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

	SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT supplier_id, item_id, item_cost, supplier_name FROM supplier", SQL_NTS);

	char supplier_id4[46], item_id4[46], supplier_name4[46];
	float item_cost4;

	SQLBindCol(handleSQL, 1, SQL_C_CHAR, supplier_id4, sizeof(supplier_id4), nullptr);
	SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_id4, sizeof(item_id4), nullptr);
	SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &item_cost4, 0, nullptr);
	SQLBindCol(handleSQL, 4, SQL_C_CHAR, supplier_name4, sizeof(supplier_name4), nullptr);

	suppliers.clear();

	while (SQLFetch(handleSQL) == SQL_SUCCESS)
	{
		suppliers.push_back({
			supplier_id4,
			item_id4,
			item_cost4,
			supplier_name4
			});
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


	struct Transaction {
		string transaction_id5;
		string item_id5;
		float item_price5;
		float tax_amount5;
		float transaction_total5;
		string transaction_date5;
	};

	string original_transaction_id5;

	vector<Transaction> transactions;

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

	SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT transaction_id, item_id, item_price, tax_amount, transaction_total, transaction_date FROM transaction", SQL_NTS);

	char transaction_id5[46], item_id5[46], transaction_date5[46];
	float item_price5, tax_amount5, transaction_total5;

	SQLBindCol(handleSQL, 1, SQL_C_CHAR, transaction_id5, sizeof(transaction_id5), nullptr);
	SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_id5, sizeof(item_id5), nullptr);
	SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &item_price5, 0, nullptr);
	SQLBindCol(handleSQL, 4, SQL_C_FLOAT, &tax_amount5, 0, nullptr);
	SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &transaction_total5, 0, nullptr);
	SQLBindCol(handleSQL, 6, SQL_C_CHAR, transaction_date5, sizeof(transaction_date5), nullptr);

	transactions.clear();

	while (SQLFetch(handleSQL) == SQL_SUCCESS)
	{
		transactions.push_back({
			transaction_id5,
			item_id5,
			item_price5,
			tax_amount5,
			transaction_total5,
			transaction_date5
			});
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	char t1In[128] = "";
	char t2In[128] = "";
	char t3In[128] = "";
	char t4In[128] = "";
	char t5In[128] = "";
	char t6In[128] = "";

/*-----------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------*/
	
	while (window.isOpen())
	{	
		while (const optional event = window.pollEvent())
		{
			ImGui::SFML::ProcessEvent(window, *event);

			if (event->is<Event::Closed>())
			{
				SQLDisconnect(dbconSQL);
				window.close();
			}
		}

		ImGui::SFML::Update(window, clock.getElapsedTime());

		if (mouseDetector.isOn(itemHeaderBox, window))
		{
			window.setMouseCursor(handCursor);

			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				clickItem = true;
				clickAisle = false;
				clickSection = false;
				clickSupplier = false;
				clickTransaction = false;

				background.setTexture(itemBackgroundTexture);
				
				textBox1.setPosition({280, 110});
				textBox2.setPosition({ 280, 158 });
				textBox3.setPosition({ 280, 206 });				
				textBox4.setPosition({ 280, 254 });
				textBox5.setPosition({ 280, 302 });
				textBox6.setPosition({ 280, 350 });

				submitButton.setPosition({ 210,400 });
				submitButton.setColor(Color::White);

				modifyButton.setPosition({ 0,0 });
				modifyButton.setColor(Color::Transparent);
			}
		}

		if (clickItem)
		{
			//t1 box
			{
				ImGui::SetNextWindowPos(ImVec2(197, 88));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##itemt1InputWindow", nullptr, ImGuiWindowFlags_NoResize
														| ImGuiWindowFlags_NoMove
														| ImGuiWindowFlags_NoCollapse
														| ImGuiWindowFlags_NoBackground
														| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##itemt1Input", t1In, sizeof(t1In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}
			
			//t2 box
			{
				ImGui::SetNextWindowPos(ImVec2(197, 136));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##itemt2InputWindow", nullptr, ImGuiWindowFlags_NoResize
														| ImGuiWindowFlags_NoMove
														| ImGuiWindowFlags_NoCollapse
														| ImGuiWindowFlags_NoBackground
														| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##itemt2Input", t2In, sizeof(t2In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t3 box
			{
				ImGui::SetNextWindowPos(ImVec2(197, 184));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##itemt3InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##itemt3Input", t3In, sizeof(t3In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t4 box
			{
				ImGui::SetNextWindowPos(ImVec2(197, 232));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##itemt4InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##itemt4Input", t4In, sizeof(t4In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t5 box
			{
				ImGui::SetNextWindowPos(ImVec2(197, 280));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##itemt5InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##itemt5Input", t5In, sizeof(t5In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t6 box
			{
				ImGui::SetNextWindowPos(ImVec2(197, 328));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##itemt6InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##itemt6Input", t6In, sizeof(t6In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}
			
			static int selectedRow = -1;
			int currentRow = 0;

			ImGui::SetNextWindowPos(ImVec2(470, 100));
			ImGui::SetNextWindowSize(ImVec2(750, 430));
			ImVec4 headerColor = ImVec4(62.0f/255.0f, 127.0f/255.0f, 8.0f/255.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

			if (ImGui::Begin("Items", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

				if (ImGui::BeginTable("##ItemsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("Item I.D.");
					ImGui::TableSetupColumn("Item Name");
					ImGui::TableSetupColumn("Aisle No.");
					ImGui::TableSetupColumn("Section I.D.");
					ImGui::TableSetupColumn("Item Price");
					ImGui::TableSetupColumn("No. of Items");
					ImGui::TableHeadersRow();

					ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
					if (sortSpecs && sortSpecs->SpecsDirty)
					{
						const ImGuiTableColumnSortSpecs* spec = &sortSpecs->Specs[0];
						int columnIndex = spec->ColumnIndex;
						bool ascending = spec->SortDirection == ImGuiSortDirection_Ascending;

						sort(items.begin(), items.end(), [&](const Item& a, const Item& b)
						{
							switch (columnIndex)
							{
							case 0: return ascending ? a.item_id1 < b.item_id1 : a.item_id1 > b.item_id1;
							case 1: return ascending ? a.item_name1 < b.item_name1 : a.item_name1 > b.item_name1;
							case 2: return ascending ? a.aisle_no1 < b.aisle_no1 : a.aisle_no1 > b.aisle_no1;
							case 3: return ascending ? a.section_id1 < b.section_id1 : a.section_id1 > b.section_id1;
							case 4: return ascending ? a.item_price1 < b.item_price1 : a.item_price1 > b.item_price1;
							case 5: return ascending ? a.no_of_items1 < b.no_of_items1 : a.no_of_items1 > b.no_of_items1;
							default: return false;
							}
						});

						sortSpecs->SpecsDirty = false;
					}

					for (const auto& item : items) 
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isSelected = (currentRow == selectedRow);

						if (ImGui::Selectable(item.item_id1.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
							selectedRow = currentRow;
						}

						ImGui::TableSetColumnIndex(1); ImGui::Text("%s", item.item_name1.c_str());
						ImGui::TableSetColumnIndex(2); ImGui::Text("%d", item.aisle_no1);
						ImGui::TableSetColumnIndex(3); ImGui::Text("%s", item.section_id1.c_str());
						ImGui::TableSetColumnIndex(4); ImGui::Text("%.2f", item.item_price1);
						ImGui::TableSetColumnIndex(5); ImGui::Text("%d", item.no_of_items1);

						currentRow++;
					}

					ImGui::EndTable();

					if (ImGui::Button("Delete"))
					{
						if (selectedRow >= 0 && selectedRow < items.size())
						{
							const string& selectedItemId = items[selectedRow].item_id1;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring deleteQuery = L"DELETE FROM item WHERE item_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							wstring item_id_wstr(selectedItemId.begin(), selectedItemId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Delete Successful" << endl;

								items.erase(items.begin() + selectedRow);
								selectedRow = -1;
							}
							else
							{
								cerr << "Delete failed" << endl;
							}
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < items.size())
						{
							submitButton.setColor(Color::Transparent);
							submitButton.setPosition({ 0,0 });
							
							modifyButton.setColor(Color::White);
							modifyButton.setPosition({ 210,400 });

							original_item_id1 = items[selectedRow].item_id1;

							strncpy_s(t1In, items[selectedRow].item_id1.c_str(), sizeof(t1In) - 1);
							t1In[sizeof(t1In) - 1] = '\0';
							strncpy_s(t2In, items[selectedRow].item_name1.c_str(), sizeof(t2In) - 1);
							t2In[sizeof(t2In) - 1] = '\0';
							snprintf(t3In, sizeof(t3In), "%d", items[selectedRow].aisle_no1);
							strncpy_s(t4In, items[selectedRow].section_id1.c_str(), sizeof(t4In) - 1);
							t4In[sizeof(t4In) - 1] = '\0';
							snprintf(t5In, sizeof(t5In), "%.2f", items[selectedRow].item_price1);
							snprintf(t6In, sizeof(t6In), "%d", items[selectedRow].no_of_items1);
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickItem)
		{
			if (mouseDetector.isOn(textBox1, window) 
			 || mouseDetector.isOn(textBox2, window) 
			 || mouseDetector.isOn(textBox3, window) 
			 || mouseDetector.isOn(textBox4, window) 
			 || mouseDetector.isOn(textBox5, window) 
			 || mouseDetector.isOn(textBox6, window)){window.setMouseCursor(textCursor);}

			if (mouseDetector.isOn(submitButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0' || t4In[0] == '\0' || t5In[0] == '\0' || t6In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atoi(t3In);
							atof(t5In);
							atoi(t6In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							string item_idStr(t1In);
							wstring item_id(item_idStr.begin(), item_idStr.end());
							string item_nameStr(t2In);
							wstring item_name(item_nameStr.begin(), item_nameStr.end());
							int aisle_no = atoi(t3In);
							string section_idStr(t4In);
							wstring section_id(section_idStr.begin(), section_idStr.end());
							float item_price = atof(t5In);
							int no_of_items = atoi(t6In);

							wstring insertQuery = L"INSERT INTO item (item_id, item_name, aisle_no, section_id, item_price, no_of_items) VALUES (?,?,?,?,?,?)";

							SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &item_price, 0, NULL);
							SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_items, 0, NULL);

							retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Insert successful!" << endl;

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';
								t4In[0] = '\0';
								t5In[0] = '\0';
								t6In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT item_id, item_name, aisle_no, section_id, item_price, no_of_items FROM item", SQL_NTS);

								char item_id[46], item_name[46], section_id[46];
								int aisle_no, no_of_items;
								float item_price;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, item_id, sizeof(item_id), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_name, sizeof(item_name), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_SLONG, &aisle_no, 0, nullptr);
								SQLBindCol(handleSQL, 4, SQL_C_CHAR, section_id, sizeof(section_id), nullptr);
								SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &item_price, 0, nullptr);
								SQLBindCol(handleSQL, 6, SQL_C_SLONG, &no_of_items, 0, nullptr);

								items.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									items.push_back({
										item_id,
										item_name,
										aisle_no,
										section_id,
										item_price,
										no_of_items
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}

							else
							{
								cerr << "Insert failed!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0' || t4In[0] == '\0' || t5In[0] == '\0' || t6In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atoi(t3In);
							atof(t5In);
							atoi(t6In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring original_item_idW(original_item_id1.begin(), original_item_id1.end());
							string item_idStr(t1In);
							wstring item_id(item_idStr.begin(), item_idStr.end());
							string item_nameStr(t2In);
							wstring item_name(item_nameStr.begin(), item_nameStr.end());
							int aisle_no = atoi(t3In);
							string section_idStr(t4In);
							wstring section_id(section_idStr.begin(), section_idStr.end());
							float item_price = atof(t5In);
							int no_of_items = atoi(t6In);

							wstring updateQuery = L"UPDATE item SET item_id = ?, item_name = ?, aisle_no = ?, section_id = ?, item_price = ?, no_of_items = ? WHERE item_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_price, 0, NULL);
							SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_items, 0, NULL);
							SQLBindParameter(handleSQL, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_item_idW.c_str(), 0, NULL);

							SQLRETURN ret = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(ret))
							{
								cout << "Item updated successfully!" << endl;

								modifyButton.setColor(Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(Color::White);
								submitButton.setPosition({ 210,400 });

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';
								t4In[0] = '\0';
								t5In[0] = '\0';
								t6In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT item_id, item_name, aisle_no, section_id, item_price, no_of_items FROM item", SQL_NTS);

								char item_id[46], item_name[46], section_id[46];
								int aisle_no, no_of_items;
								float item_price;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, item_id, sizeof(item_id), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_name, sizeof(item_name), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_SLONG, &aisle_no, 0, nullptr);
								SQLBindCol(handleSQL, 4, SQL_C_CHAR, section_id, sizeof(section_id), nullptr);
								SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &item_price, 0, nullptr);
								SQLBindCol(handleSQL, 6, SQL_C_SLONG, &no_of_items, 0, nullptr);

								items.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									items.push_back({
										item_id,
										item_name,
										aisle_no,
										section_id,
										item_price,
										no_of_items
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}
							else
							{
								cerr << "Failed to update item!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}
		}

		if (mouseDetector.isOn(aisleHeaderBox, window))
		{
			window.setMouseCursor(handCursor);

			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				clickItem = false;
				clickAisle = true;
				clickSection = false;
				clickSupplier = false;
				clickTransaction = false;

				background.setTexture(aisleBackgroundTexture);

				textBox1.setPosition({ 310, 110 });
				textBox2.setPosition({ 310, 158 });

				submitButton.setPosition({ 210,220 });
				submitButton.setColor(Color::White);

				modifyButton.setPosition({ 0,0 });
				modifyButton.setColor(Color::Transparent);
			}
		}

		if (clickAisle)
		{
			//t1 box
			{
				ImGui::SetNextWindowPos(ImVec2(227, 88));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 50)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##aislet1InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##aislet1Input", t1In, sizeof(t1In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t2 box
			{
				ImGui::SetNextWindowPos(ImVec2(227, 136));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##aislet2InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##aislet2Input", t2In, sizeof(t2In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}
			
			static int selectedRow = -1;
			int currentRow = 0;

			ImGui::SetNextWindowPos(ImVec2(470, 100));
			ImGui::SetNextWindowSize(ImVec2(750, 430));
			ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

			if (ImGui::Begin("Aisles", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

				if (ImGui::BeginTable("##AislesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("Aisle No.");
					ImGui::TableSetupColumn("No. of Sections");
					ImGui::TableHeadersRow();

					ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
					if (sortSpecs && sortSpecs->SpecsDirty)
					{
						const ImGuiTableColumnSortSpecs* spec = &sortSpecs->Specs[0];
						int columnIndex = spec->ColumnIndex;
						bool ascending = spec->SortDirection == ImGuiSortDirection_Ascending;

						sort(aisles.begin(), aisles.end(), [&](const Aisle& a, const Aisle& b)
							{
								switch (columnIndex)
								{
								case 0: return ascending ? a.aisle_no2 < b.aisle_no2 : a.aisle_no2 > b.aisle_no2;
								case 1: return ascending ? a.no_of_sections2 < b.no_of_sections2 : a.no_of_sections2 > b.no_of_sections2;
								default: return false;
								}
							});

						sortSpecs->SpecsDirty = false;
					}

					for (const auto& aisle : aisles)
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isSelected = (currentRow == selectedRow);
						string aisleStr = to_string(aisle.aisle_no2);
						if (ImGui::Selectable(aisleStr.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
							selectedRow = currentRow;
						}

						ImGui::TableSetColumnIndex(1); ImGui::Text("%d", aisle.no_of_sections2);

						currentRow++;
					}

					ImGui::EndTable();

					if (ImGui::Button("Delete"))
					{
						if (selectedRow >= 0 && selectedRow < aisles.size())
						{
							const string& selectedAisleNo = to_string(aisles[selectedRow].aisle_no2);

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring deleteQuery = L"DELETE FROM aisle WHERE aisle_no = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							wstring aisle_no_wstr(selectedAisleNo.begin(), selectedAisleNo.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)aisle_no_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Delete Successful" << endl;

								aisles.erase(aisles.begin() + selectedRow);
								selectedRow = -1;
							}
							else
							{
								cerr << "Delete failed" << endl;
							}
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < aisles.size())
						{
							submitButton.setColor(Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(Color::White);
							modifyButton.setPosition({ 210,220 });

							original_aisle_no2 = aisles[selectedRow].aisle_no2;

							snprintf(t1In, sizeof(t2In), "%d", aisles[selectedRow].aisle_no2);
							snprintf(t2In, sizeof(t2In), "%d", aisles[selectedRow].no_of_sections2);
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickAisle)
		{
			if (mouseDetector.isOn(textBox1, window) || mouseDetector.isOn(textBox2, window)) {window.setMouseCursor(textCursor);}

			if (mouseDetector.isOn(submitButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atoi(t1In);
							atoi(t2In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							int aisle_no = atoi(t1In);
							int no_of_sections = atoi(t2In);

							wstring insertQuery = L"INSERT INTO aisle (aisle_no, no_of_sections) VALUES (?,?)";

							SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_sections, 0, NULL);

							retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Insert successful!" << endl;

								t1In[0] = '\0';
								t2In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT aisle_no, no_of_sections FROM aisle", SQL_NTS);

								int aisle_no2, no_of_sections2;

								SQLBindCol(handleSQL, 1, SQL_C_SLONG, &aisle_no2, 0, nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_SLONG, &no_of_sections2, 0, nullptr);

								aisles.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									aisles.push_back({
										aisle_no2,
										no_of_sections2
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}

							else
							{
								cerr << "Insert failed!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atoi(t1In);
							atoi(t2In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							int original_aisle_no = original_aisle_no2;
							int aisle_no = atoi(t1In);
							int no_of_sections = atoi(t2In);

							wstring updateQuery = L"UPDATE aisle SET aisle_no = ?, no_of_sections = ? WHERE aisle_no = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_sections, 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &original_aisle_no, 0, NULL);

							SQLRETURN ret = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(ret))
							{
								cout << "Item updated successfully!" << endl;

								modifyButton.setColor(Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(Color::White);
								submitButton.setPosition({ 210,220 });

								t1In[0] = '\0';
								t2In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT aisle_no, no_of_sections FROM aisle", SQL_NTS);

								int aisle_no2, no_of_sections2;

								SQLBindCol(handleSQL, 1, SQL_C_SLONG, &aisle_no2, 0, nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_SLONG, &no_of_sections2, 0, nullptr);

								aisles.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									aisles.push_back({
										aisle_no2,
										no_of_sections2
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}
							else
							{
								cerr << "Failed to update item!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}
		}

		if (mouseDetector.isOn(sectionHeaderBox, window))
		{
			window.setMouseCursor(handCursor);

			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				clickItem = false;
				clickAisle = false;
				clickSection = true;
				clickSupplier = false;
				clickTransaction = false;

				background.setTexture(sectionBackgroundTexture);

				textBox1.setPosition({ 300, 110 });
				textBox2.setPosition({ 300, 158 });
				textBox3.setPosition({ 300, 206 });

				submitButton.setPosition({ 210,250 });
				submitButton.setColor(Color::White);

				modifyButton.setPosition({ 0,0 });
				modifyButton.setColor(Color::Transparent);
			}
		}

		if (clickSection)
		{
			//t1 box
			{
				ImGui::SetNextWindowPos(ImVec2(217, 88));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##sectiont1InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##sectiont1Input", t1In, sizeof(t1In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t2 box
			{
				ImGui::SetNextWindowPos(ImVec2(217, 136));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##sectiont2InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##sectiont2Input", t2In, sizeof(t2In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t3 box
			{
				ImGui::SetNextWindowPos(ImVec2(217, 184));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##sectiont3InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##sectiont3Input", t3In, sizeof(t3In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			static int selectedRow = -1;
			int currentRow = 0;

			ImGui::SetNextWindowPos(ImVec2(470, 100));
			ImGui::SetNextWindowSize(ImVec2(750, 430));
			ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

			if (ImGui::Begin("Sections", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

				if (ImGui::BeginTable("##SectionsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("Section I.D.");
					ImGui::TableSetupColumn("Section Name");
					ImGui::TableSetupColumn("Aisle No.");
					ImGui::TableHeadersRow();

					ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
					if (sortSpecs && sortSpecs->SpecsDirty)
					{
						const ImGuiTableColumnSortSpecs* spec = &sortSpecs->Specs[0];
						int columnIndex = spec->ColumnIndex;
						bool ascending = spec->SortDirection == ImGuiSortDirection_Ascending;

						sort(sections.begin(), sections.end(), [&](const Section& a, const Section& b)
							{
								switch (columnIndex)
								{
								case 0: return ascending ? a.section_id3 < b.section_id3 : a.section_id3 > b.section_id3;
								case 1: return ascending ? a.section_name3 < b.section_name3 : a.section_name3 > b.section_name3;
								case 2: return ascending ? a.aisle_no3 < b.aisle_no3 : a.aisle_no3 > b.aisle_no3;
								default: return false;
								}
							});

						sortSpecs->SpecsDirty = false;
					}

					for (const auto& section : sections)
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isSelected = (currentRow == selectedRow);
						
						if (ImGui::Selectable(section.section_id3.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
							selectedRow = currentRow;
						}

						ImGui::TableSetColumnIndex(1); ImGui::Text("%s", section.section_name3.c_str());
						ImGui::TableSetColumnIndex(2); ImGui::Text("%d", section.aisle_no3);

						currentRow++;
					}

					ImGui::EndTable();

					if (ImGui::Button("Delete"))
					{
						if (selectedRow >= 0 && selectedRow < sections.size())
						{
							const string& selectedSectionId = sections[selectedRow].section_id3;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring deleteQuery = L"DELETE FROM section WHERE section_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							wstring section_id_wstr(selectedSectionId.begin(), selectedSectionId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Delete Successful" << endl;

								sections.erase(sections.begin() + selectedRow);
								selectedRow = -1;
							}
							else
							{
								cerr << "Delete failed" << endl;
							}
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < sections.size())
						{
							submitButton.setColor(Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(Color::White);
							modifyButton.setPosition({ 210,250 });

							original_section_id3 = sections[selectedRow].section_id3;

							strncpy_s(t1In, sections[selectedRow].section_id3.c_str(), sizeof(t1In) - 1);
							t1In[sizeof(t1In) - 1] = '\0';
							strncpy_s(t2In, sections[selectedRow].section_name3.c_str(), sizeof(t2In) - 1);
							t2In[sizeof(t2In) - 1] = '\0';
							snprintf(t3In, sizeof(t3In), "%d", sections[selectedRow].aisle_no3);
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickSection)
		{
			if (mouseDetector.isOn(textBox1, window) || mouseDetector.isOn(textBox2, window) || mouseDetector.isOn(textBox3, window)){window.setMouseCursor(textCursor);}

			if (mouseDetector.isOn(submitButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{

					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{

						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atoi(t3In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							string section_idStr(t1In);
							wstring section_id(section_idStr.begin(), section_idStr.end());
							string section_nameStr(t2In);
							wstring section_name(section_nameStr.begin(), section_nameStr.end());
							int aisle_no = atoi(t3In);

							wstring insertQuery = L"INSERT INTO section (section_id, section_name, aisle_no) VALUES (?,?,?)";

							SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);

							retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Insert successful!" << endl;

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT section_id, section_name, aisle_no FROM section", SQL_NTS);

								char section_id3[46], section_name3[46];
								int aisle_no3;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, section_id3, sizeof(section_id3), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, section_name3, sizeof(section_name3), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_SLONG, &aisle_no3, 0, nullptr);

								sections.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									sections.push_back({
										section_id3,
										section_name3,
										aisle_no3
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}

							else
							{
								cerr << "Insert failed!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atoi(t3In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring original_section_id(original_section_id3.begin(), original_section_id3.end());
							string section_idStr(t1In);
							wstring section_id(section_idStr.begin(), section_idStr.end());
							string section_nameStr(t2In);
							wstring section_name(section_nameStr.begin(), section_nameStr.end());
							int aisle_no = atoi(t3In);;

							wstring updateQuery = L"UPDATE section SET section_id = ?, section_name = ?, aisle_no = ? WHERE section_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_section_id.c_str(), 0, NULL);

							SQLRETURN ret = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(ret))
							{
								cout << "Item updated successfully!" << endl;

								modifyButton.setColor(Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(Color::White);
								submitButton.setPosition({ 210,250 });

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT section_id, section_name, aisle_no FROM section", SQL_NTS);

								char section_id3[46], section_name3[46];
								int aisle_no3;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, section_id3, sizeof(section_id3), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, section_name3, sizeof(section_name3), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_SLONG, &aisle_no3, 0, nullptr);

								sections.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									sections.push_back({
										section_id3,
										section_name3,
										aisle_no3
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}
							else
							{
								cerr << "Failed to update item!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}
		}

		if (mouseDetector.isOn(supplierHeaderBox, window))
		{
			window.setMouseCursor(handCursor);

			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				clickItem = false;
				clickAisle = false;
				clickSection = false;
				clickSupplier = true;
				clickTransaction = false;

				background.setTexture(supplierBackgroundTexture);

				textBox1.setPosition({ 310, 110 });
				textBox2.setPosition({ 310, 158 });
				textBox3.setPosition({ 310, 206 });
				textBox4.setPosition({ 310, 254 });

				submitButton.setPosition({ 210,300 });
				submitButton.setColor(Color::White);

				modifyButton.setPosition({ 0,0 });
				modifyButton.setColor(Color::Transparent);
			}
		}

		if (clickSupplier)
		{
			//t1 box
			{
				ImGui::SetNextWindowPos(ImVec2(227, 88));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##suppliert1InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##suppliert1Input", t1In, sizeof(t1In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t2 box
			{
				ImGui::SetNextWindowPos(ImVec2(227, 136));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##suppliert2InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##suppliert2Input", t2In, sizeof(t2In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t3 box
			{
				ImGui::SetNextWindowPos(ImVec2(227, 184));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##suppliert3InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##suppliert3Input", t3In, sizeof(t3In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t4 box
			{
				ImGui::SetNextWindowPos(ImVec2(227, 232));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##suppliert4InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##suppliert4Input", t4In, sizeof(t4In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			static int selectedRow = -1;
			int currentRow = 0;

			ImGui::SetNextWindowPos(ImVec2(470, 100));
			ImGui::SetNextWindowSize(ImVec2(750, 430));
			ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

			if (ImGui::Begin("Suppliers", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

				if (ImGui::BeginTable("##SuppliersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("Supplier I.D.");
					ImGui::TableSetupColumn("Item I.D.");
					ImGui::TableSetupColumn("Item Cost");
					ImGui::TableSetupColumn("Supplier Name");
					ImGui::TableHeadersRow();

					ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
					if (sortSpecs && sortSpecs->SpecsDirty)
					{
						const ImGuiTableColumnSortSpecs* spec = &sortSpecs->Specs[0];
						int columnIndex = spec->ColumnIndex;
						bool ascending = spec->SortDirection == ImGuiSortDirection_Ascending;

						sort(suppliers.begin(), suppliers.end(), [&](const Supplier& a, const Supplier& b)
							{
								switch (columnIndex)
								{
								case 0: return ascending ? a.supplier_id4 < b.supplier_id4 : a.supplier_id4 > b.supplier_id4;
								case 1: return ascending ? a.item_id4 < b.item_id4 : a.item_id4 > b.item_id4;
								case 2: return ascending ? a.item_cost4 < b.item_cost4 : a.item_cost4 > b.item_cost4;
								case 3: return ascending ? a.supplier_name4 < b.supplier_name4 : a.supplier_name4 > b.supplier_name4;
								default: return false;
								}
							});

						sortSpecs->SpecsDirty = false;
					}

					for (const auto& supplier : suppliers)
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isSelected = (currentRow == selectedRow);

						if (ImGui::Selectable(supplier.supplier_id4.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
							selectedRow = currentRow;
						}

						ImGui::TableSetColumnIndex(1); ImGui::Text("%s", supplier.item_id4.c_str());
						ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", supplier.item_cost4);
						ImGui::TableSetColumnIndex(3); ImGui::Text("%s", supplier.supplier_name4.c_str());

						currentRow++;
					}

					ImGui::EndTable();

					if (ImGui::Button("Delete"))
					{
						if (selectedRow >= 0 && selectedRow < suppliers.size())
						{
							const string& selectedSupplierId = suppliers[selectedRow].supplier_id4;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring deleteQuery = L"DELETE FROM supplier WHERE supplier_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							wstring supplier_id_wstr(selectedSupplierId.begin(), selectedSupplierId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Delete Successful" << endl;

								suppliers.erase(suppliers.begin() + selectedRow);
								selectedRow = -1;
							}
							else
							{
								cerr << "Delete failed" << endl;
							}
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < suppliers.size())
						{
							submitButton.setColor(Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(Color::White);
							modifyButton.setPosition({ 210,300 });

							original_supplier_id4 = suppliers[selectedRow].supplier_id4;

							strncpy_s(t1In, suppliers[selectedRow].supplier_id4.c_str(), sizeof(t1In) - 1);
							t1In[sizeof(t1In) - 1] = '\0';
							strncpy_s(t2In, suppliers[selectedRow].item_id4.c_str(), sizeof(t2In) - 1);
							t2In[sizeof(t2In) - 1] = '\0';
							snprintf(t3In, sizeof(t3In), "%.2f", suppliers[selectedRow].item_cost4);
							strncpy_s(t4In, suppliers[selectedRow].supplier_name4.c_str(), sizeof(t4In) - 1);
							t4In[sizeof(t4In) - 1] = '\0';
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickSupplier)
		{
			if (mouseDetector.isOn(textBox1, window) || mouseDetector.isOn(textBox2, window)|| mouseDetector.isOn(textBox3, window) || mouseDetector.isOn(textBox4, window)){window.setMouseCursor(textCursor);}

			if (mouseDetector.isOn(submitButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0' || t4In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atof(t3In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							string supplier_idStr(t1In);
							wstring supplier_id(supplier_idStr.begin(), supplier_idStr.end());
							string item_idStr(t2In);
							wstring item_id(item_idStr.begin(), item_idStr.end());
							float item_cost = atof(t3In);
							string supplier_nameStr(t4In);
							wstring supplier_name(supplier_nameStr.begin(), supplier_nameStr.end());

							wstring insertQuery = L"INSERT INTO supplier (supplier_id, item_id, item_cost, supplier_name) VALUES (?,?,?,?)";

							SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_cost, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_name.c_str(), 0, NULL);

							retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Insert successful!" << endl;

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';
								t4In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT supplier_id, item_id, item_cost, supplier_name FROM supplier", SQL_NTS);

								char supplier_id4[46], item_id4[46], supplier_name4[46];
								float item_cost4;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, supplier_id4, sizeof(supplier_id4), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_id4, sizeof(item_id4), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &item_cost4, 0, nullptr);
								SQLBindCol(handleSQL, 4, SQL_C_CHAR, supplier_name4, sizeof(supplier_name4), nullptr);

								suppliers.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									suppliers.push_back({
										supplier_id4,
										item_id4,
										item_cost4,
										supplier_name4
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}

							else
							{
								cerr << "Insert failed!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0' || t4In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atof(t3In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring original_supplier_id(original_supplier_id4.begin(), original_supplier_id4.end());
							string supplier_idStr(t1In);
							wstring supplier_id(supplier_idStr.begin(), supplier_idStr.end());
							string item_idStr(t2In);
							wstring item_id(item_idStr.begin(), item_idStr.end());
							float item_cost = atof(t3In);
							string supplier_nameStr(t4In);
							wstring supplier_name(supplier_nameStr.begin(), supplier_nameStr.end());

							wstring updateQuery = L"UPDATE supplier SET supplier_id = ?, item_id = ?, item_cost = ?, supplier_name = ? WHERE supplier_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_cost, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_supplier_id.c_str(), 0, NULL);

							retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Item updated successfully!" << endl;

								modifyButton.setColor(Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(Color::White);
								submitButton.setPosition({ 210,300 });

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';
								t4In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT supplier_id, item_id, item_cost, supplier_name FROM supplier", SQL_NTS);

								char supplier_id4[46], item_id4[46], supplier_name4[46];
								float item_cost4;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, supplier_id4, sizeof(supplier_id4), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_id4, sizeof(item_id4), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &item_cost4, 0, nullptr);
								SQLBindCol(handleSQL, 4, SQL_C_CHAR, supplier_name4, sizeof(supplier_name4), nullptr);

								suppliers.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									suppliers.push_back({
										supplier_id4,
										item_id4,
										item_cost4,
										supplier_name4
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}
							else
							{
								cerr << "Failed to update item!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}
		}

		if (mouseDetector.isOn(transactionHeaderBox, window))
		{
			window.setMouseCursor(handCursor);

			if (Mouse::isButtonPressed(Mouse::Button::Left))
			{
				clickItem = false;
				clickAisle = false;
				clickSection = false;
				clickSupplier = false;
				clickTransaction = true;

				background.setTexture(transactionBackgroundTexture);

				textBox1.setPosition({ 330, 110 });
				textBox2.setPosition({ 330, 158 });
				textBox3.setPosition({ 330, 206 });
				textBox4.setPosition({ 330, 254 });
				textBox5.setPosition({ 330, 302 });
				textBox6.setPosition({ 330, 350 });

				submitButton.setPosition({ 210,400 });
				submitButton.setColor(Color::White);

				modifyButton.setPosition({ 0,0 });
				modifyButton.setColor(Color::Transparent);
			}
		}

		if (clickTransaction)
		{
			//t1 box
			{
				ImGui::SetNextWindowPos(ImVec2(247, 88));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##transactiont1InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##transactiont1Input", t1In, sizeof(t1In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t2 box
			{
				ImGui::SetNextWindowPos(ImVec2(247, 136));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##transactiont2InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##transactiont2Input", t2In, sizeof(t2In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t3 box
			{
				ImGui::SetNextWindowPos(ImVec2(247, 184));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##transactiont3InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##transactiont3Input", t3In, sizeof(t3In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t4 box
			{
				ImGui::SetNextWindowPos(ImVec2(247, 232));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##transactiont4InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##transactiont4Input", t4In, sizeof(t4In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t5 box
			{
				ImGui::SetNextWindowPos(ImVec2(247, 280));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##transactiont5InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##transactiont5Input", t5In, sizeof(t5In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			//t6 box
			{
				ImGui::SetNextWindowPos(ImVec2(247, 328));//x-offset:8	y-offset:+11
				ImGui::SetNextWindowSize(ImVec2(231, 45)); //x-offset:81
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(105.f / 255.f, 106.f / 255.f, 106.f / 255.f, 1.f));
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.f, 0.f, 0.f, 1.f));
				ImGui::PushFont(guiFont);
				ImGui::Begin("##transactiont6InputWindow", nullptr, ImGuiWindowFlags_NoResize
					| ImGuiWindowFlags_NoMove
					| ImGuiWindowFlags_NoCollapse
					| ImGuiWindowFlags_NoBackground
					| ImGuiWindowFlags_NoTitleBar);
				ImGui::InputText("##transactiont6Input", t6In, sizeof(t6In));
				ImGui::PopStyleColor(2);
				ImGui::PopFont();
				ImGui::End();
			}

			static int selectedRow = -1;
			int currentRow = 0;

			ImGui::SetNextWindowPos(ImVec2(470, 100));
			ImGui::SetNextWindowSize(ImVec2(750, 430));
			ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

			if (ImGui::Begin("Transactions", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
			{
				ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

				if (ImGui::BeginTable("##TransactionsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
				{
					ImGui::TableSetupColumn("Transaction I.D.");
					ImGui::TableSetupColumn("Item I.D.");
					ImGui::TableSetupColumn("Item Price");
					ImGui::TableSetupColumn("Tax Amount");
					ImGui::TableSetupColumn("Transaction Total");
					ImGui::TableSetupColumn("Transaction Date");
					ImGui::TableHeadersRow();

					ImGuiTableSortSpecs* sortSpecs = ImGui::TableGetSortSpecs();
					if (sortSpecs && sortSpecs->SpecsDirty)
					{
						const ImGuiTableColumnSortSpecs* spec = &sortSpecs->Specs[0];
						int columnIndex = spec->ColumnIndex;
						bool ascending = spec->SortDirection == ImGuiSortDirection_Ascending;

						sort(transactions.begin(), transactions.end(), [&](const Transaction& a, const Transaction& b)
							{
								switch (columnIndex)
								{
								case 0: return ascending ? a.transaction_id5 < b.transaction_id5 : a.transaction_id5 > b.transaction_id5;
								case 1: return ascending ? a.item_id5 < b.item_id5 : a.item_id5 > b.item_id5;
								case 2: return ascending ? a.item_price5 < b.item_price5 : a.item_price5 > b.item_price5;
								case 3: return ascending ? a.tax_amount5 < b.tax_amount5 : a.tax_amount5 > b.tax_amount5;
								case 4: return ascending ? a.transaction_total5 < b.transaction_total5 : a.transaction_total5 > b.transaction_total5;
								case 5: return ascending ? a.transaction_date5 < b.transaction_date5 : a.transaction_date5 > b.transaction_date5;
								default: return false;
								}
							});

						sortSpecs->SpecsDirty = false;
					}

					for (const auto& transaction : transactions)
					{
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						bool isSelected = (currentRow == selectedRow);

						if (ImGui::Selectable(transaction.transaction_id5.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
							selectedRow = currentRow;
						}

						ImGui::TableSetColumnIndex(1); ImGui::Text("%s", transaction.item_id5.c_str());
						ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", transaction.item_price5);
						ImGui::TableSetColumnIndex(3); ImGui::Text("%.2f", transaction.tax_amount5);
						ImGui::TableSetColumnIndex(4); ImGui::Text("%.2f", transaction.transaction_total5);						
						ImGui::TableSetColumnIndex(5); ImGui::Text("%s", transaction.transaction_date5.c_str());

						currentRow++;
					}

					ImGui::EndTable();

					if (ImGui::Button("Delete"))
					{
						if (selectedRow >= 0 && selectedRow < transactions.size())
						{
							const string& selectedTransactionId = transactions[selectedRow].transaction_id5;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring deleteQuery = L"DELETE FROM transaction WHERE transaction_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							wstring transaction_id_wstr(selectedTransactionId.begin(), selectedTransactionId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Delete Successful" << endl;

								transactions.erase(transactions.begin() + selectedRow);
								selectedRow = -1;
							}
							else
							{
								cerr << "Delete failed" << endl;
							}
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < transactions.size())
						{
							submitButton.setColor(Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(Color::White);
							modifyButton.setPosition({ 210,400 });

							original_transaction_id5 = transactions[selectedRow].transaction_id5;

							strncpy_s(t1In, transactions[selectedRow].transaction_id5.c_str(), sizeof(t1In) - 1);
							t1In[sizeof(t1In) - 1] = '\0';
							strncpy_s(t2In, transactions[selectedRow].item_id5.c_str(), sizeof(t2In) - 1);
							t2In[sizeof(t2In) - 1] = '\0';
							snprintf(t3In, sizeof(t3In), "%.2f", transactions[selectedRow].item_price5);
							snprintf(t4In, sizeof(t4In), "%.2f", transactions[selectedRow].tax_amount5);
							snprintf(t5In, sizeof(t5In), "%.2f", transactions[selectedRow].transaction_total5);
							strncpy_s(t6In, transactions[selectedRow].transaction_date5.c_str(), sizeof(t6In) - 1);
							t6In[sizeof(t6In) - 1] = '\0';
						}
					}
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickTransaction)
		{
			if (mouseDetector.isOn(textBox1, window)
				|| mouseDetector.isOn(textBox2, window)
				|| mouseDetector.isOn(textBox3, window)
				|| mouseDetector.isOn(textBox4, window)
				|| mouseDetector.isOn(textBox5, window)
				|| mouseDetector.isOn(textBox6, window)) {window.setMouseCursor(textCursor);}

			if (mouseDetector.isOn(submitButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0' || t4In[0] == '\0' || t5In[0] == '\0' || t6In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atof(t3In);
							atof(t4In);
							atof(t5In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							string transaction_idStr(t1In);
							wstring transaction_id(transaction_idStr.begin(), transaction_idStr.end());
							string item_idStr(t2In);
							wstring item_id(item_idStr.begin(), item_idStr.end());
							float item_price = atof(t3In);
							float tax_amount = atof(t4In);
							float transaction_total = atof(t5In);
							string transaction_dateStr(t6In);
							wstring transaction_date(transaction_dateStr.begin(), transaction_dateStr.end());

							wstring insertQuery = L"INSERT INTO transaction (transaction_id, item_id, item_price, tax_amount, transaction_total, transaction_date) VALUES (?,?,?,?,?,?)";

							SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &item_price, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &tax_amount, 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &transaction_total, 0, NULL);
							SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_date.c_str(), 0, NULL);

							retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Insert successful!" << endl;

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';
								t4In[0] = '\0';
								t5In[0] = '\0';
								t6In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT transaction_id, item_id, item_price, tax_amount, transaction_total, transaction_date FROM transaction", SQL_NTS);

								char transaction_id5[46], item_id5[46], transaction_date5[46];
								float item_price5, tax_amount5, transaction_total5;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, transaction_id5, sizeof(transaction_id5), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_id5, sizeof(item_id5), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &item_price5, 0, nullptr);
								SQLBindCol(handleSQL, 4, SQL_C_FLOAT, &tax_amount5, 0, nullptr);
								SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &transaction_total5, 0, nullptr);
								SQLBindCol(handleSQL, 6, SQL_C_CHAR, transaction_date5, sizeof(transaction_date5), nullptr);

								transactions.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									transactions.push_back({
										transaction_id5,
										item_id5,
										item_price5,
										tax_amount5,
										transaction_total5,
										transaction_date5
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}

							else
							{
								cerr << "Insert failed!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				window.setMouseCursor(handCursor);

				if (Mouse::isButtonPressed(Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1In[0] == '\0' || t2In[0] == '\0' || t3In[0] == '\0' || t4In[0] == '\0' || t5In[0] == '\0' || t6In[0] == '\0')
						{
							cerr << "Empty inputs detected" << endl;
							notNull = false;
						}

						try
						{
							atof(t3In);
							atof(t4In);
							atof(t5In);
						}
						catch (const exception& e)
						{
							cerr << "invalid numbers" << endl;
							valNums = false;
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							wstring original_transaction_idW(original_transaction_id5.begin(), original_transaction_id5.end());
							string transaction_idStr(t1In);
							wstring transaction_id(transaction_idStr.begin(), transaction_idStr.end());
							string item_idStr(t2In);
							wstring item_id(item_idStr.begin(), item_idStr.end());
							float item_price = atof(t3In);
							float tax_amount = atof(t4In);
							float transaction_total = atof(t5In);
							string transaction_dateStr(t6In);
							wstring transaction_date(transaction_dateStr.begin(), transaction_dateStr.end());

							wstring updateQuery = L"UPDATE transaction SET transaction_id = ?, item_id = ?, item_price = ?, tax_amount = ?, transaction_total = ?, transaction_date = ? WHERE transaction_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_price, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &tax_amount, 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &transaction_total, 0, NULL);
							SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_date.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_transaction_idW.c_str(), 0, NULL);


							retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								cout << "Item updated successfully!" << endl;

								modifyButton.setColor(Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(Color::White);
								submitButton.setPosition({ 210,400 });

								t1In[0] = '\0';
								t2In[0] = '\0';
								t3In[0] = '\0';
								t4In[0] = '\0';
								t5In[0] = '\0';
								t6In[0] = '\0';

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SELECT transaction_id, item_id, item_price, tax_amount, transaction_total, transaction_date FROM transaction", SQL_NTS);

								char transaction_id5[46], item_id5[46], transaction_date5[46];
								float item_price5, tax_amount5, transaction_total5;

								SQLBindCol(handleSQL, 1, SQL_C_CHAR, transaction_id5, sizeof(transaction_id5), nullptr);
								SQLBindCol(handleSQL, 2, SQL_C_CHAR, item_id5, sizeof(item_id5), nullptr);
								SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &item_price5, 0, nullptr);
								SQLBindCol(handleSQL, 4, SQL_C_FLOAT, &tax_amount5, 0, nullptr);
								SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &transaction_total5, 0, nullptr);
								SQLBindCol(handleSQL, 6, SQL_C_CHAR, transaction_date5, sizeof(transaction_date5), nullptr);

								transactions.clear();

								while (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									transactions.push_back({
										transaction_id5,
										item_id5,
										item_price5,
										tax_amount5,
										transaction_total5,
										transaction_date5
										});
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}
							else
							{
								cerr << "Failed to update item!" << endl;
							}

							notNull = true;
							valNums = true;
						}
						clock.restart();
					}
				}
			}
		}

		if (mouseDetector.isOn(background, window) && !mouseDetector.isOn(submitButton, window)
												   && !mouseDetector.isOn(modifyButton, window)
												   && !mouseDetector.isOn(textBox1, window)
												   && !mouseDetector.isOn(textBox2, window)
												   && !mouseDetector.isOn(textBox3, window)
												   && !mouseDetector.isOn(textBox4, window)
												   && !mouseDetector.isOn(textBox5, window)
												   && !mouseDetector.isOn(textBox6, window)
												   && !mouseDetector.isOn(itemHeaderBox, window)
												   && !mouseDetector.isOn(aisleHeaderBox, window)
												   && !mouseDetector.isOn(sectionHeaderBox, window)
												   && !mouseDetector.isOn(supplierHeaderBox, window)
												   && !mouseDetector.isOn(transactionHeaderBox, window)) {window.setMouseCursor(arrowCursor);}

		window.clear();
		window.draw(background);
		window.draw(submitButton);
		window.draw(modifyButton);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
}