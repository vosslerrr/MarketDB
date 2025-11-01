#include <windows.h>
#include <iostream>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <imgui-SFML.h>
#include "MouseDetector.h"
#include "ImGuiObject.h"

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

	sf::RenderWindow loginwindow = sf::RenderWindow(sf::VideoMode({ 800,600 }), "Log In", sf::Style::Close);
	loginwindow.setFramerateLimit(60);

	ImGui::SFML::Init(loginwindow);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontFromFileTTF("arial.ttf", 20.f);
	ImGui::SFML::UpdateFontTexture();
	io.FontDefault = io.Fonts->Fonts.back();

	sf::Texture loginBackgroundTexture;
	loginBackgroundTexture.loadFromFile("loginColumns.png");
	sf::Sprite loginBackground(loginBackgroundTexture);

	sf::Clock loginClock;

	ImGuiObject serverBox;
	ImGuiObject portBox;
	ImGuiObject databaseBox;
	ImGuiObject uidBox;
	ImGuiObject pwdBox;
	ImGuiObject logInButton;

	while (loginwindow.isOpen())
	{
		while (const std::optional event = loginwindow.pollEvent())
		{
			ImGui::SFML::ProcessEvent(loginwindow, *event);

			if (event->is<sf::Event::Closed>())
			{
				loginwindow.close();
				//return 0;
			}
		}

		ImGui::SFML::Update(loginwindow, loginClock.restart());

		serverBox.drawTextBox(ImVec2(317, 279), "##ServerInputWindow", "##ServerInput");
		portBox.drawTextBox(ImVec2(317, 319), "##PortInputWindow", "##PortInput");
		databaseBox.drawTextBox(ImVec2(317, 359), "##DatabaseInputWindow", "##DatabaseInput");
		uidBox.drawTextBox(ImVec2(317, 399), "##UIDInputWindow", "##UIDInput");
		pwdBox.drawTextBox(ImVec2(317, 439), "##PWDInputWindow", "##PWDInput");
		logInButton.drawButton(ImVec2(360, 475), "##logInButton", "Log In");

		if (logInButton.isPressed())
		{
			std::string serverInStr(serverBox.getInput());
			std::wstring wServerInput(serverInStr.begin(), serverInStr.end());
			std::string portInStr(portBox.getInput());
			std::wstring wPortInput(portInStr.begin(), portInStr.end());
			std::string databaseInStr(databaseBox.getInput());
			std::wstring wDatabaseInput(databaseInStr.begin(), databaseInStr.end());
			std::string uidInStr(uidBox.getInput());
			std::wstring wUidInput(uidInStr.begin(), uidInStr.end());
			std::string pwdInStr(pwdBox.getInput());
			std::wstring wPwdInput(pwdInStr.begin(), pwdInStr.end());
			std::wstring connStr = L"DRIVER={MySQL ODBC 8.0 ANSI Driver};SERVER=" + wServerInput + L";PORT=" + wPortInput + L";DATABASE=" +
				wDatabaseInput + L";UID=" + wUidInput + L";PWD=" + wPwdInput + L";";

			retSQL = SQLDriverConnect(dbconSQL, NULL, (SQLWCHAR*)connStr.c_str(), SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE);

			if (SQL_SUCCEEDED(retSQL))
			{
				std::cout << "Connected" << std::endl;

				SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
				SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
				SQLFreeHandle(SQL_HANDLE_DBC, dbconSQL);
				SQLFreeHandle(SQL_HANDLE_ENV, envSQL);

				loginwindow.close();
			}

			else
			{
				int error = MessageBoxW(
					nullptr,
					L"One or more of the inputs are incorrect.\n"
					L"Connection to MySQL has failed.\n\n"
					L"Please try again.",
					L"Connection Failed!",
					MB_OK | MB_ICONWARNING
				);
			}

			logInButton.setPressedOff();
		}

		loginwindow.clear();
		loginwindow.draw(loginBackground);
		ImGui::SFML::Render(loginwindow);
		loginwindow.display();
	}

	bool itemTable = false;
	bool aisleTable = false;
	bool sectionTable = false;
	bool supplierTable = false;
	bool transactionTable = false;

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'aisle'", SQL_NTS);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			aisleTable = true;
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (!aisleTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE `aisle` ("
			L"`aisle_no` int NOT NULL,"
			L"`no_of_sections` int NOT NULL,"
			L"PRIMARY KEY(`aisle_no`)"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'section'", SQL_NTS);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			sectionTable = true;
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (!sectionTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE `section` ("
			L"`section_id` varchar(45) NOT NULL,"
			L"`section_name` varchar(45) NOT NULL,"
			L"`aisle_no` int NOT NULL,"
			L"PRIMARY KEY(`section_id`),"
			L"KEY `aisle_no_idx` (`aisle_no`),"
			L"CONSTRAINT `fk_section_aisle` FOREIGN KEY(`aisle_no`) REFERENCES `aisle` (`aisle_no`)"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
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
			L"CREATE TABLE `item` ("
			L"`item_id` varchar(45) NOT NULL,"
			L"`item_name` varchar(45) NOT NULL,"
			L"`aisle_no` int NOT NULL,"
			L"`section_id` varchar(45) NOT NULL,"
			L"`item_price` float NOT NULL,"
			L"`no_of_items` int NOT NULL,"
			L"PRIMARY KEY(`item_id`),"
			L"KEY `fk_item_aisle` (`aisle_no`),"
			L"KEY `fk_item_section` (`section_id`),"
			L"CONSTRAINT `fk_item_aisle` FOREIGN KEY(`aisle_no`) REFERENCES `aisle` (`aisle_no`),"
			L"CONSTRAINT `fk_item_section` FOREIGN KEY(`section_id`) REFERENCES `section` (`section_id`)"
			L");";

		retSQL = SQLExecDirect(handleSQL, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'supplier'", SQL_NTS);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			supplierTable = true;
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (!supplierTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE `supplier` ("
			L"`supplier_id` varchar(45) NOT NULL,"
			L"`item_id` varchar(45) NOT NULL,"
			L"`item_cost` float NOT NULL,"
			L"`supplier_name` varchar(45) NOT NULL,"
			L"PRIMARY KEY(`supplier_id`),"
			L"KEY `fk_supplier_item` (`item_id`),"
			L"CONSTRAINT `fk_supplier_item` FOREIGN KEY(`item_id`) REFERENCES `item` (`item_id`)"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
	}

	SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
	retSQL = SQLExecDirectW(handleSQL, (SQLWCHAR*)L"SHOW TABLES LIKE 'transaction'", SQL_NTS);

	if (SQL_SUCCEEDED(retSQL)) {
		if (SQLFetch(handleSQL) == SQL_SUCCESS) {
			transactionTable = true;
		}
	}

	SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

	if (!transactionTable)
	{
		SQLHSTMT hCreateStmt;
		SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &hCreateStmt);

		const wchar_t* createQuery =
			L"CREATE TABLE `transaction` ("
			L"`transaction_id` varchar(45) NOT NULL,"
			L"`item_id` varchar(45) NOT NULL,"
			L"`item_price` float NOT NULL,"
			L"`tax_amount` float NOT NULL,"
			L"`transaction_total` float NOT NULL,"
			L"`transaction_date` varchar(45) NOT NULL,"
			L"PRIMARY KEY(`transaction_id`),"
			L"KEY `fk_transaction_item` (`item_id`),"
			L"CONSTRAINT `fk_transaction_item` FOREIGN KEY(`item_id`) REFERENCES `item` (`item_id`)"
			L")";

		SQLRETURN createRet = SQLExecDirect(hCreateStmt, (SQLWCHAR*)createQuery, SQL_NTS);

		SQLFreeHandle(SQL_HANDLE_STMT, hCreateStmt);
	}

/*----------------------------------------------------------------------------------------------------------------------------------------------- 
-------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------*/
	
	sf::RenderWindow window = sf::RenderWindow(sf::VideoMode({ 1280,720 }), "MarketDB", sf::Style::Close);
	window.setFramerateLimit(60);

	ImGui::SFML::Init(window);
	ImGuiIO& io2 = ImGui::GetIO();
	io2.Fonts->AddFontFromFileTTF("arial.ttf", 20.f);
	ImGui::SFML::UpdateFontTexture();
	io2.FontDefault = io2.Fonts->Fonts.back();
	ImFont* headerFont = io2.Fonts->AddFontFromFileTTF("arial.ttf", 36.f);
	ImGui::SFML::UpdateFontTexture();

	sf::Texture backgroundTexture;
	backgroundTexture.loadFromFile("backgrounds.png");
	sf::Sprite currBackground{ backgroundTexture };
	sf::IntRect backgrounds[6];

	for (int i = 0; i < 6; i++)
	{
		backgrounds[i] = sf::IntRect({ {1280 * i, 0}, {1280,720} });
	}

	currBackground.setTextureRect(backgrounds[0]);

	ImGuiObject itemButton{  headerFont };
	ImGuiObject aisleButton{ headerFont };
	ImGuiObject sectionButton{ headerFont };
	ImGuiObject supplierButton{ headerFont };
	ImGuiObject transactionButton{ headerFont };

	sf::RectangleShape searchButton({44,43});
	searchButton.setOrigin({ searchButton.getGeometricCenter().x, searchButton.getGeometricCenter().y });
	searchButton.setPosition({1170,24});

	sf::Texture submitButtonTexture;
	submitButtonTexture.loadFromFile("buttonBox.png");
	sf::Sprite submitButton(submitButtonTexture);
	submitButton.setOrigin({ 24,8 });
	submitButton.setPosition({ 0,0 });
	submitButton.setColor(sf::Color::Transparent);

	sf::Texture modifyButtonTexture;
	modifyButtonTexture.loadFromFile("modifyBox.png");
	sf::Sprite modifyButton(modifyButtonTexture);
	modifyButton.setOrigin({ 24,8 });
	modifyButton.setPosition({0,0});
	modifyButton.setColor(sf::Color::Transparent);

	MouseDetector mouseDetector;
	sf::Clock clock;
	bool notNull = true;
	bool valNums = true;
	bool clickItem = false;
	bool clickAisle = false;
	bool clickSection = false;
	bool clickSupplier = false;
	bool clickTransaction = false;
	bool clickSearch = false;
	bool clickSelection = false;
	bool valT1 = true;
	bool valT2 = true;
	bool valT3 = true;
	bool valT4 = true;
	bool valT5 = true;
	bool valT6 = true;
	bool primaryKeyIsVal = true;
	bool clickGO = false;
	bool showSearchItem = false;
	bool showSearchAisle = false;
	bool showSearchSection = false;
	bool showSearchSupplier = false;
	bool showSearchTransaction = false;
	bool valAisleNo = true;
	bool valSectionID = true;
	bool valItemID = true;

	struct Item {
		std::string item_id1;
		std::string item_name1;
		int aisle_no1;
		std::string section_id1;
		float item_price1;
		int no_of_items1;
	};

	std::string original_item_id1;

	std::vector<Item> items;

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

	std::vector<Aisle> aisles;

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
		std::string section_id3;
		std::string section_name3;
		int aisle_no3;
	};

	std::string original_section_id3;

	std::vector<Section> sections;

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
		std::string supplier_id4;
		std::string item_id4;
		float item_cost4;
		std::string supplier_name4;
	};

	std::string original_supplier_id4;

	std::vector<Supplier> suppliers;

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
		std::string transaction_id5;
		std::string item_id5;
		float item_price5;
		float tax_amount5;
		float transaction_total5;
		std::string transaction_date5;
	};

	std::string original_transaction_id5;

	std::vector<Transaction> transactions;

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

	// search item construction
	struct SearchItem {
		std::string search_item_id1;
		std::string search_item_name1;
		int search_aisle_no1;
		std::string search_section_id1;
		float search_item_price1;
		int search_no_of_items1;
	};

	std::vector<SearchItem> searchitems;

	char search_item_id1[46], search_item_name1[46], search_section_id1[46];
	int search_aisle_no1, search_no_of_items1;
	float search_item_price1;
	
	// search aisle construction
	struct SearchAisle {
		int search_aisle_no2;
		int search_no_of_sections2;
	};

	std::vector<SearchAisle> searchaisles;

	int search_aisle_no2, search_no_of_sections2;

	// search section construction
	struct SearchSection {
		std::string search_section_id3;
		std::string search_section_name3;
		int search_aisle_no3;
	};

	std::vector<SearchSection> searchsections;

	char search_section_id3[46], search_section_name3[46];
	int search_aisle_no3;

	// search supplier construction
	struct SearchSupplier {
		std::string search_supplier_id4;
		std::string search_item_id4;
		float search_item_cost4;
		std::string search_supplier_name4;
	};

	std::vector<SearchSupplier> searchsuppliers;

	char search_supplier_id4[46], search_item_id4[46], search_supplier_name4[46];
	float search_item_cost4;

	// search transaction construction
	struct SearchTransaction {
		std::string search_transaction_id5;
		std::string search_item_id5;
		float search_item_price5;
		float search_tax_amount5;
		float search_transaction_total5;
		std::string search_transaction_date5;
	};

	std::vector<SearchTransaction> searchtransactions;

	char search_transaction_id5[46], search_item_id5[46], search_transaction_date5[46];
	float search_item_price5, search_tax_amount5, search_transaction_total5;

	ImGuiObject t1Box;
	ImGuiObject t2Box;
	ImGuiObject t3Box;
	ImGuiObject t4Box;
	ImGuiObject t5Box;
	ImGuiObject t6Box;
	
/*-----------------------------------------------------------------------------------------------------------------------------------------------
-------------------------------------------------------------------------------------------------------------------------------------------------
-----------------------------------------------------------------------------------------------------------------------------------------------*/
	
	while (window.isOpen())
	{	
		while (const std::optional event = window.pollEvent())
		{
			ImGui::SFML::ProcessEvent(window, *event);

			if (event->is<sf::Event::Closed>())
			{
				SQLDisconnect(dbconSQL);
				window.close();
			}
		}

		ImGui::SFML::Update(window, clock.restart());

		itemButton.drawHeaderButton(ImVec2(69, 0), "##itemButton", "Item");
		aisleButton.drawHeaderButton(ImVec2(190, 0), "##aisleButton", "Aisle");
		sectionButton.drawHeaderButton(ImVec2(324, 0), "##sectionButton", "Section");
		supplierButton.drawHeaderButton(ImVec2(493, 0), "##supplierButton", "Supplier");
		transactionButton.drawHeaderButton(ImVec2(670, 0), "##transactionButton", "Transaction");

		if (itemButton.isPressed())
		{
			clickItem = true;
			clickAisle = false;
			clickSection = false;
			clickSupplier = false;
			clickTransaction = false;
			clickSearch = false;
			clickGO = false;

			currBackground.setTextureRect(backgrounds[1]);

			t1Box.setValid();
			t2Box.setValid();
			t3Box.setValid();
			t4Box.setValid();
			t5Box.setValid();
			t6Box.setValid();

			submitButton.setPosition({ 210,400 });
			submitButton.setColor(sf::Color::White);

			modifyButton.setPosition({ 0,0 });
			modifyButton.setColor(sf::Color::Transparent);

			itemButton.setPressedOff();
		}

		if (clickItem)
		{	
			t1Box.drawTextBox(ImVec2(197,88), "##itemT1InputWindow", "##itemT1Input");
			t2Box.drawTextBox(ImVec2(197,136), "##itemT2InputWindow", "##itemT2Input");
			t3Box.drawTextBox(ImVec2(197,184), "##itemT3InputWindow", "##itemT3Input");
			t4Box.drawTextBox(ImVec2(197, 232), "##itemT4InputWindow", "##itemT4Input");
			t5Box.drawTextBox(ImVec2(197,280), "##itemT5InputWindow", "##itemT5Input");
			t6Box.drawTextBox(ImVec2(197,328), "##itemT6InputWindow", "##itemT6Input");
			
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
							const std::string& selectedItemId = items[selectedRow].item_id1;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring deleteQuery = L"DELETE FROM item WHERE item_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							std::wstring item_id_wstr(selectedItemId.begin(), selectedItemId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								std::cout << "Delete Successful" << std::endl;

								items.erase(items.begin() + selectedRow);
								selectedRow = -1;

								submitButton.setPosition({ 210,400 });
								submitButton.setColor(sf::Color::White);

								modifyButton.setPosition({ 0,0 });
								modifyButton.setColor(sf::Color::Transparent);
							}
							else
							{
								std::cerr  << "Delete failed" << std::endl;
							}
						}
					}

					//TODO: Fix this idk what this even does
					/*
					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < items.size())
						{
							submitButton.setColor(sf::Color::Transparent);
							submitButton.setPosition({ 0,0 });
							
							modifyButton.setColor(sf::Color::White);
							modifyButton.setPosition({ 210,400 });

							original_item_id1 = items[selectedRow].item_id1;

							strncpy_s(t1Box.getInput(), items[selectedRow].item_id1.c_str(), sizeof(t1Box.getInput()) - 1);
							t1Box.getInput()[sizeof(t1Box.getInput()) - 1] = '\0';
							strncpy_s(t2In, items[selectedRow].item_name1.c_str(), sizeof(t2In) - 1);
							t2In[sizeof(t2In) - 1] = '\0';
							snprintf(t3In, sizeof(t3In), "%d", items[selectedRow].aisle_no1);
							strncpy_s(t4In, items[selectedRow].section_id1.c_str(), sizeof(t4In) - 1);
							t4In[sizeof(t4In) - 1] = '\0';
							snprintf(t5In, sizeof(t5In), "%.2f", items[selectedRow].item_price1);
							snprintf(t6In, sizeof(t6In), "%d", items[selectedRow].no_of_items1);
						}
					}
					*/
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickItem)
		{
			if (mouseDetector.isOn(submitButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t3Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						if (t4Box.getInput() == "")
						{
							std::cerr  << "Empty t4" << std::endl;
							notNull = false;

							t4Box.setInvalid();
						}
						else
						{
							t4Box.setValid();
						}

						if (t5Box.getInput() == "")
						{
							std::cerr  << "Empty t5" << std::endl;
							notNull = false;

							t5Box.setInvalid();
						}
						else
						{
							t5Box.setValid();
						}

						if (t6Box.getInput() == "")
						{
							std::cerr  << "Empty t6" << std::endl;
							notNull = false;

							t6Box.setInvalid();
						}
						else
						{
							t6Box.setValid();
						}

						try
						{
							std::string t3InTEST(t3Box.getInput());
							
							std::stoi(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t3Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						try
						{
							std::string t5InTEST(t5Box.getInput());

							std::stof(t5InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t5 number" << std::endl;
							valNums = false;
							valT5 = false;

							t5Box.setInvalid();
						}
						if (valT5)
						{
							t5Box.setValid();
						}

						try
						{
							std::string t6InTEST(t6Box.getInput());

							std::stoi(t6InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t6 number" << std::endl;
							valNums = false;
							valT6 = false;

							t6Box.setInvalid();
						}
						if (valT6)
						{
							t6Box.setValid();
						}

						if (notNull && valNums)
						{
							std::string item_idStr(t1Box.getInput());
							std::wstring item_id(item_idStr.begin(), item_idStr.end());

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
							SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item_id FROM item WHERE item_id = ?", SQL_NTS);
							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
							retSQL = SQLExecute(handleSQL);

							if (retSQL == SQL_SUCCESS || retSQL == SQL_SUCCESS_WITH_INFO) 
							{
								if (SQLFetch(handleSQL) == SQL_SUCCESS) 
								{
									std::cout << "Item ID already exists!" << std::endl;
									primaryKeyIsVal = false;

									t1Box.setInvalid();

									int primaryKeyError = MessageBoxW(
										nullptr,
										L"Item I.D. is already in use.\n\n"
										L"Please try again.",
										L"Duplicate Primary Key!",
										MB_OK | MB_ICONWARNING
									);
								}
								else 
								{
									primaryKeyIsVal = true;

									t1Box.setValid();
								}
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (primaryKeyIsVal)
							{
								std::string item_nameStr(t2Box.getInput());
								std::wstring item_name(item_nameStr.begin(), item_nameStr.end());

								int aisle_no = atoi(t3Box.getInput());

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
								SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT aisle_no FROM aisle WHERE aisle_no = ?", SQL_NTS);
								SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, nullptr);
								retSQL = SQLExecute(handleSQL);

								if (retSQL == SQL_SUCCESS)
								{
									if (SQLFetch(handleSQL) != SQL_SUCCESS)
									{
										std::cout << "aisle no does not exist" << std::endl;
										valAisleNo = false;

										t3Box.setInvalid();

										int foreignKeyAisleError = MessageBoxW(
											nullptr,
											L"Aisle No. does not exist.\n\n"
											L"Please try again.",
											L"Foreign Key Constraint!",
											MB_OK | MB_ICONWARNING
										);
									}
									else
									{
										valAisleNo = true;

										t3Box.setValid();
									}
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

								if (valAisleNo)
								{
									std::string section_idStr(t4Box.getInput());
									std::wstring section_id(section_idStr.begin(), section_idStr.end());

									SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
									SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT section_id FROM section WHERE section_id = ?", SQL_NTS);
									SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)section_id.c_str(), 0, nullptr);
									retSQL = SQLExecute(handleSQL);

									if (retSQL == SQL_SUCCESS)
									{
										if (SQLFetch(handleSQL) != SQL_SUCCESS)
										{
											std::cout << "section id does not exist" << std::endl;
											valSectionID = false;

											t4Box.setInvalid();

											int foreignKeyAisleError = MessageBoxW(
												nullptr,
												L"Section I.D. does not exist.\n\n"
												L"Please try again.",
												L"Foreign Key Constraint!",
												MB_OK | MB_ICONWARNING
											);
										}
										else
										{
											valSectionID = true;

											t4Box.setValid();
										}
									}

									SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

									if (valSectionID)
									{
										float item_price = atof(t5Box.getInput());
										int no_of_items = atoi(t6Box.getInput());

										std::wstring insertQuery = L"INSERT INTO item (item_id, item_name, aisle_no, section_id, item_price, no_of_items) VALUES (?,?,?,?,?,?)";

										SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

										SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

										SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
										SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_name.c_str(), 0, NULL);
										SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
										SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
										SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &item_price, 0, NULL);
										SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_items, 0, NULL);

										retSQL = SQLExecute(handleSQL);

										if (SQL_SUCCEEDED(retSQL))
										{
											std::cout << "Insert successful!" << std::endl;

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
											std::cerr  << "Insert failed!" << std::endl;
										}

										SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
									}
								}
							}
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT1 = true;
						valT2 = true;
						valT3 = true;
						valT4 = true;
						valT5 = true;
						valT6 = true;

						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						if (t4Box.getInput() == "")
						{
							std::cerr  << "Empty t4" << std::endl;
							notNull = false;

							t4Box.setInvalid();
						}
						else
						{
							t4Box.setValid();
						}

						if (t5Box.getInput() == "")
						{
							std::cerr  << "Empty t5" << std::endl;
							notNull = false;

							t5Box.setInvalid();
						}
						else
						{
							t5Box.setInvalid();
						}

						if (t6Box.getInput() == "")
						{
							std::cerr  << "Empty t6" << std::endl;
							notNull = false;

							t6Box.setInvalid();
						}
						else
						{
							t6Box.setValid();
						}

						try
						{
							std::string t3InTEST(t3Box.getInput());

							std::stoi(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t2Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						try
						{
							std::string t5InTEST(t5Box.getInput());

							std::stof(t5InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t5 number" << std::endl;
							valNums = false;
							valT5 = false;

							t5Box.setInvalid();
						}
						if (valT5)
						{
							t5Box.setInvalid();
						}

						try
						{
							std::string t6InTEST(t6Box.getInput());

							std::stoi(t6InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t6 number" << std::endl;
							valNums = false;
							valT6 = false;

							t6Box.setInvalid();
						}
						if (valT6)
						{
							t6Box.setValid();
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring original_item_idW(original_item_id1.begin(), original_item_id1.end());
							std::string item_idStr(t1Box.getInput());
							std::wstring item_id(item_idStr.begin(), item_idStr.end());
							std::string item_nameStr(t2Box.getInput());
							std::wstring item_name(item_nameStr.begin(), item_nameStr.end());
							int aisle_no = atoi(t3Box.getInput());
							std::string section_idStr(t4Box.getInput());
							std::wstring section_id(section_idStr.begin(), section_idStr.end());
							float item_price = atof(t5Box.getInput());
							int no_of_items = atoi(t6Box.getInput());

							std::wstring updateQuery = L"UPDATE item SET item_id = ?, item_name = ?, aisle_no = ?, section_id = ?, item_price = ?, no_of_items = ? WHERE item_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_price, 0, NULL);
							SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_items, 0, NULL);
							SQLBindParameter(handleSQL, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_item_idW.c_str(), 0, NULL);

							SQLRETURN ret = SQLExecute(handleSQL);

							if (SQL_SUCCEEDED(ret))
							{
								std::cout << "Item updated successfully!" << std::endl;

								modifyButton.setColor(sf::Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(sf::Color::White);
								submitButton.setPosition({ 210,400 });

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
								std::cerr  << "Failed to update item!" << std::endl;
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT1 = true;
						valT2 = true;
						valT3 = true;
						valT4 = true;
						valT5 = true;
						valT6 = true;

						clock.restart();
					}
				}
			}
		}

		if (aisleButton.isPressed())
		{
			clickItem = false;
			clickAisle = true;
			clickSection = false;
			clickSupplier = false;
			clickTransaction = false;
			clickSearch = false;
			clickGO = false;

			currBackground.setTextureRect(backgrounds[2]);
				
			t1Box.setValid();
			t2Box.setValid();
			t3Box.setValid();
			t4Box.setValid();
			t5Box.setValid();
			t6Box.setValid();

			submitButton.setPosition({ 210,220 });
			submitButton.setColor(sf::Color::White);

			modifyButton.setPosition({ 0,0 });
			modifyButton.setColor(sf::Color::Transparent);

			aisleButton.setPressedOff();
		}

		if (clickAisle)
		{
			t1Box.drawTextBox(ImVec2(227,88), "##aisleT1InputWindow", "##aisleT1Input");
			t2Box.drawTextBox(ImVec2(227,136), "##aisleT2InputWindow", "##aisleT2Input");
			
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
						std::string aisleStr = std::to_string(aisle.aisle_no2);
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
							const std::string& selectedAisleNo = std::to_string(aisles[selectedRow].aisle_no2);

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring deleteQuery = L"DELETE FROM aisle WHERE aisle_no = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							std::wstring aisle_no_wstr(selectedAisleNo.begin(), selectedAisleNo.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)aisle_no_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								std::cout << "Delete Successful" << std::endl;

								aisles.erase(aisles.begin() + selectedRow);
								selectedRow = -1;

								submitButton.setPosition({ 210,220 });
								submitButton.setColor(sf::Color::White);

								modifyButton.setPosition({ 0,0 });
								modifyButton.setColor(sf::Color::Transparent);
							}
							else
							{
								std::cerr  << "Delete failed" << std::endl;
							}
						}
					}

					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < aisles.size())
						{
							submitButton.setColor(sf::Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(sf::Color::White);
							modifyButton.setPosition({ 210,220 });

							original_aisle_no2 = aisles[selectedRow].aisle_no2;

							snprintf(t1Box.getInput(), sizeof(t2Box.getInput()), "%d", aisles[selectedRow].aisle_no2);
							snprintf(t2Box.getInput(), sizeof(t2Box.getInput()), "%d", aisles[selectedRow].no_of_sections2);
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
			if (mouseDetector.isOn(submitButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							notNull = true;
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							notNull = true;
							t2Box.setValid();
						}

						try
						{
							std::string t1InTEST(t1Box.getInput());

							std::stoi(t1InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t1 number" << std::endl;
							valT1 = false;
							valNums = false;

							t1Box.setInvalid();
						}
						if (valT1)
						{
							t1Box.setValid();
						}

						try
						{
							std::string t2InTEST(t2Box.getInput());

							std::stoi(t2InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t2 number" << std::endl;
							valT2 = false;
							valNums = false;

							t2Box.setInvalid();
						}
						if (valT2)
						{
							t2Box.setValid();
						}

						if (notNull && valNums)
						{
							int aisle_no = atoi(t1Box.getInput());

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
							SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT aisle_no FROM aisle WHERE aisle_no = ?", SQL_NTS);
							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							retSQL = SQLExecute(handleSQL);

							if (retSQL == SQL_SUCCESS || retSQL == SQL_SUCCESS_WITH_INFO)
							{
								if (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									std::cout << "Aisle No. already exists!" << std::endl;
									primaryKeyIsVal = false;

									t1Box.setInvalid();

									int primaryKeyError = MessageBoxW(
										nullptr,
										L"Aisle No. is already in use.\n\n"
										L"Please try again.",
										L"Duplicate Primary Key!",
										MB_OK | MB_ICONWARNING
									);
								}
								else
								{
									primaryKeyIsVal = true;

									t1Box.setValid();
								}
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (primaryKeyIsVal)
							{
								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

								int no_of_sections = atoi(t2Box.getInput());

								std::wstring insertQuery = L"INSERT INTO aisle (aisle_no, no_of_sections) VALUES (?,?)";

								SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

								SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
								SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_sections, 0, NULL);

								retSQL = SQLExecute(handleSQL);

								if (SQL_SUCCEEDED(retSQL))
								{
									std::cout << "Insert successful!" << std::endl;

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
									std::cerr  << "Insert failed!" << std::endl;
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
							}
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						valT1 = true;
						valT2 = true;
						notNull = true;
						valNums = true;

						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						try
						{
							std::string t1InTEST(t1Box.getInput());

							std::stoi(t1InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t1 number" << std::endl;
							valNums = false;
							valT1 = false;

							t1Box.setInvalid();
						}
						if (valT1)
						{
							t1Box.setValid();
						}

						try
						{
							std::string t2InTEST(t2Box.getInput());

							std::stoi(t2InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t2 number" << std::endl;
							valNums = false;
							valT2 = false;

							t2Box.setInvalid();
						}
						if (valT2)
						{
							t2Box.setValid();
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							int original_aisle_no = original_aisle_no2;
							int aisle_no = atoi(t1Box.getInput());
							int no_of_sections = atoi(t2Box.getInput());

							std::wstring updateQuery = L"UPDATE aisle SET aisle_no = ?, no_of_sections = ? WHERE aisle_no = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &no_of_sections, 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &original_aisle_no, 0, NULL);

							SQLRETURN ret = SQLExecute(handleSQL);

							if (SQL_SUCCEEDED(ret))
							{
								std::cout << "Item updated successfully!" << std::endl;

								modifyButton.setColor(sf::Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(sf::Color::White);
								submitButton.setPosition({ 210,220 });

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
								std::cerr  << "Failed to update item!" << std::endl;
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
						}

						else 
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						valT1 = true;
						valT2 = true;
						notNull = true;
						valNums = true;

						clock.restart();
					}
				}
			}
		}

		if (sectionButton.isPressed())
		{
			clickItem = false;
			clickAisle = false;
			clickSection = true;
			clickSupplier = false;
			clickTransaction = false;
			clickSearch = false;
			clickGO = false;

			currBackground.setTextureRect(backgrounds[3]);

			t1Box.setValid();
			t2Box.setValid();
			t3Box.setValid();
			t4Box.setValid();
			t5Box.setValid();
			t6Box.setValid();

			submitButton.setPosition({ 210,250 });
			submitButton.setColor(sf::Color::White);

			modifyButton.setPosition({ 0,0 });
			modifyButton.setColor(sf::Color::Transparent);

			sectionButton.setPressedOff();
		}

		if (clickSection)
		{
			t1Box.drawTextBox(ImVec2(217,88), "##sectionT1InputWindow", "##sectionT1Input");
			t2Box.drawTextBox(ImVec2(217,136), "##sectionT2InputWindow", "##sectionT2Input");
			t3Box.drawTextBox(ImVec2(217,184), "##sectionT3InputWindow", "##sectionT3Input");

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
							const std::string& selectedSectionId = sections[selectedRow].section_id3;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring deleteQuery = L"DELETE FROM section WHERE section_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							std::wstring section_id_wstr(selectedSectionId.begin(), selectedSectionId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								std::cout << "Delete Successful" << std::endl;

								sections.erase(sections.begin() + selectedRow);
								selectedRow = -1;

								submitButton.setPosition({ 210,250 });
								submitButton.setColor(sf::Color::White);

								modifyButton.setPosition({ 0,0 });
								modifyButton.setColor(sf::Color::Transparent);
							}
							else
							{
								std::cerr  << "Delete failed" << std::endl;
							}
						}
					}
					//TODO: Fix this idk what this even does
					/*
					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < sections.size())
						{
							submitButton.setColor(sf::Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(sf::Color::White);
							modifyButton.setPosition({ 210,250 });

							original_section_id3 = sections[selectedRow].section_id3;

							strncpy_s(t1Box.getInput(), sections[selectedRow].section_id3.c_str(), sizeof(t1Box.getInput()) - 1);
							t1Box.getInput()[sizeof(t1Box.getInput()) - 1] = '\0';
							strncpy_s(t2Box.getInput(), sections[selectedRow].section_name3.c_str(), sizeof(t2Box.getInput()) - 1);
							t2Box.getInput()[sizeof(t1Box.getInput()) - 1] = '\0';
							snprintf(t3Box.getInput(), sizeof(t3Box.getInput()), "%d", sections[selectedRow].aisle_no3);
						}
					}
					*/
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickSection)
		{
			if (mouseDetector.isOn(submitButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{

					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{

						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						try
						{
							std::string t3InTEST(t3Box.getInput());

							std::stoi(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t2Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						if (notNull && valNums)
						{
							std::string section_idStr(t1Box.getInput());
							std::wstring section_id(section_idStr.begin(), section_idStr.end());

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
							SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT section_id FROM section WHERE section_id = ?", SQL_NTS);
							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)section_id.c_str(), 0, nullptr);
							retSQL = SQLExecute(handleSQL);

							if (retSQL == SQL_SUCCESS || retSQL == SQL_SUCCESS_WITH_INFO)
							{
								if (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									std::cout << "Section ID already exists!" << std::endl;
									primaryKeyIsVal = false;

									t1Box.setInvalid();

									int primaryKeyError = MessageBoxW(
										nullptr,
										L"Section I.D. is already in use.\n\n"
										L"Please try again.",
										L"Duplicate Primary Key!",
										MB_OK | MB_ICONWARNING
									);
								}
								else
								{
									primaryKeyIsVal = true;

									t1Box.setValid();
								}
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (primaryKeyIsVal)
							{
								std::string section_nameStr(t2Box.getInput());
								std::wstring section_name(section_nameStr.begin(), section_nameStr.end());
								int aisle_no = std::atoi(t3Box.getInput());

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
								SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT aisle_no FROM aisle WHERE aisle_no = ?", SQL_NTS);
								SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, nullptr);
								retSQL = SQLExecute(handleSQL);

								if (retSQL == SQL_SUCCESS)
								{
									if (SQLFetch(handleSQL) != SQL_SUCCESS)
									{
										std::cout << "aisle no does not exist" << std::endl;
										valAisleNo = false;

										t2Box.setInvalid();

										int foreignKeyAisleError = MessageBoxW(
											nullptr,
											L"Aisle No. does not exist.\n\n"
											L"Please try again.",
											L"Foreign Key Constraint!",
											MB_OK | MB_ICONWARNING
										);
									}
									else
									{
										valAisleNo = true;

										t3Box.setValid();
									}
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

								if (valAisleNo)
								{
									std::wstring insertQuery = L"INSERT INTO section (section_id, section_name, aisle_no) VALUES (?,?,?)";

									SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

									SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

									SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
									SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_name.c_str(), 0, NULL);
									SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);

									retSQL = SQLExecute(handleSQL);

									if (SQL_SUCCEEDED(retSQL))
									{
										std::cout << "Insert successful!" << std::endl;

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
										std::cerr  << "Insert failed!" << std::endl;
									}

									SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
								}
							}		
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT3 = true;

						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						try
						{
							std::string t3InTEST(t2Box.getInput());

							std::stoi(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t2Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring original_section_id(original_section_id3.begin(), original_section_id3.end());
							std::string section_idStr(t1Box.getInput());
							std::wstring section_id(section_idStr.begin(), section_idStr.end());
							std::string section_nameStr(t2Box.getInput());
							std::wstring section_name(section_nameStr.begin(), section_nameStr.end());
							int aisle_no = atoi(t3Box.getInput());;

							std::wstring updateQuery = L"UPDATE section SET section_id = ?, section_name = ?, aisle_no = ? WHERE section_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)section_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_SLONG, SQL_INTEGER, 0, 0, &aisle_no, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_section_id.c_str(), 0, NULL);

							SQLRETURN ret = SQLExecute(handleSQL);

							if (SQL_SUCCEEDED(ret))
							{
								std::cout << "Item updated successfully!" << std::endl;

								modifyButton.setColor(sf::Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(sf::Color::White);
								submitButton.setPosition({ 210,250 });

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
								std::cerr  << "Failed to update item!" << std::endl;
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT3 = true;

						clock.restart();
					}
				}
			}
		}

		if (supplierButton.isPressed())
		{
			clickItem = false;
			clickAisle = false;
			clickSection = false;
			clickSupplier = true;
			clickTransaction = false;
			clickSearch = false;
			clickGO = false;

			currBackground.setTextureRect(backgrounds[4]);

			t1Box.setValid();
			t2Box.setValid();
			t3Box.setValid();
			t4Box.setValid();
			t5Box.setValid();
			t6Box.setValid();

			submitButton.setPosition({ 210,300 });
			submitButton.setColor(sf::Color::White);

			modifyButton.setPosition({ 0,0 });
			modifyButton.setColor(sf::Color::Transparent);

			supplierButton.setPressedOff();
		}

		if (clickSupplier)
		{
			t1Box.drawTextBox(ImVec2(227,88), "##supplierT1InputWindow", "##supplierT1Input");
			t2Box.drawTextBox(ImVec2(227,136), "##supplierT2InputWindow", "##supplierT2Input");
			t3Box.drawTextBox(ImVec2(227,184), "##supplierT3InputWindow", "##supplierT3Input");
			t4Box.drawTextBox(ImVec2(227,232), "##supplierT4InputWindow", "##suuplierT4Input");

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
							const std::string& selectedSupplierId = suppliers[selectedRow].supplier_id4;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring deleteQuery = L"DELETE FROM supplier WHERE supplier_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							std::wstring supplier_id_wstr(selectedSupplierId.begin(), selectedSupplierId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);
							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								std::cout << "Delete Successful" << std::endl;

								suppliers.erase(suppliers.begin() + selectedRow);
								selectedRow = -1;

								submitButton.setPosition({ 210,300 });
								submitButton.setColor(sf::Color::White);

								modifyButton.setPosition({ 0,0 });
								modifyButton.setColor(sf::Color::Transparent);
							}
							else
							{
								std::cerr  << "Delete failed" << std::endl;
							}
						}
					}
					//TODO fix this idk what it even does
					/*
					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < suppliers.size())
						{
							submitButton.setColor(sf::Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(sf::Color::White);
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
					*/
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickSupplier)
		{
			if (mouseDetector.isOn(submitButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						if (t4Box.getInput() == "")
						{
							std::cerr  << "Empty t4" << std::endl;
							notNull = false;

							t4Box.setInvalid();
						}
						else
						{
							t4Box.setValid();
						}

						try
						{
							std::string t3InTEST(t3Box.getInput());

							std::stof(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t2Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						if (notNull && valNums)
						{
							std::string supplier_idStr(t1Box.getInput());
							std::wstring supplier_id(supplier_idStr.begin(), supplier_idStr.end());

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
							SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT supplier_id FROM supplier WHERE supplier_id = ?", SQL_NTS);
							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)supplier_id.c_str(), 0, nullptr);
							retSQL = SQLExecute(handleSQL);

							if (retSQL == SQL_SUCCESS || retSQL == SQL_SUCCESS_WITH_INFO)
							{
								if (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									std::cout << "Supplier ID already exists!" << std::endl;
									primaryKeyIsVal = false;

									t1Box.setInvalid();

									int primaryKeyError = MessageBoxW(
										nullptr,
										L"Supplier I.D. is already in use.\n\n"
										L"Please try again.",
										L"Duplicate Primary Key!",
										MB_OK | MB_ICONWARNING
									);
								}
								else
								{
									primaryKeyIsVal = true;

									t1Box.setValid();
								}
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


							if (primaryKeyIsVal)
							{
								std::string item_idStr(t2Box.getInput());
								std::wstring item_id(item_idStr.begin(), item_idStr.end());

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
								SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item_id FROM item WHERE item_id = ?", SQL_NTS);
								SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
								retSQL = SQLExecute(handleSQL);

								if (retSQL == SQL_SUCCESS || retSQL == SQL_SUCCESS_WITH_INFO)
								{
									if (SQLFetch(handleSQL) != SQL_SUCCESS)
									{
										std::cout << "Item ID does exists!" << std::endl;
										valItemID = false;

										t2Box.setInvalid();

										int primaryKeyError = MessageBoxW(
											nullptr,
											L"Item I.D. does not exist.\n\n"
											L"Please try again.",
											L"Foreign Key Constraint",
											MB_OK | MB_ICONWARNING
										);
									}
									else
									{
										valItemID = true;

										t2Box.setValid();
									}
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

								if (valItemID)
								{
									float item_cost = atof(t3Box.getInput());
									std::string supplier_nameStr(t4Box.getInput());
									std::wstring supplier_name(supplier_nameStr.begin(), supplier_nameStr.end());

									SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

									std::wstring insertQuery = L"INSERT INTO supplier (supplier_id, item_id, item_cost, supplier_name) VALUES (?,?,?,?)";

									SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

									SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_id.c_str(), 0, NULL);
									SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
									SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_cost, 0, NULL);
									SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_name.c_str(), 0, NULL);

									retSQL = SQLExecute(handleSQL);

									if (SQL_SUCCEEDED(retSQL))
									{
										std::cout << "Insert successful!" << std::endl;

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
										std::cerr  << "Insert failed!" << std::endl;
									}

									SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
								}

								
							}
							
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT3 = true;

						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						if (t4Box.getInput() == "")
						{
							std::cerr  << "Empty t4" << std::endl;
							notNull = false;

							t4Box.setInvalid();
						}
						else
						{
							t4Box.setValid();
						}

						try
						{
							std::string t3InTEST(t3Box.getInput());

							std::stof(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t2Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring original_supplier_id(original_supplier_id4.begin(), original_supplier_id4.end());
							std::string supplier_idStr(t1Box.getInput());
							std::wstring supplier_id(supplier_idStr.begin(), supplier_idStr.end());
							std::string item_idStr(t2Box.getInput());
							std::wstring item_id(item_idStr.begin(), item_idStr.end());
							float item_cost = atof(t3Box.getInput());
							std::string supplier_nameStr(t4Box.getInput());
							std::wstring supplier_name(supplier_nameStr.begin(), supplier_nameStr.end());

							std::wstring updateQuery = L"UPDATE supplier SET supplier_id = ?, item_id = ?, item_cost = ?, supplier_name = ? WHERE supplier_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_cost, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)supplier_name.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_supplier_id.c_str(), 0, NULL);

							retSQL = SQLExecute(handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								std::cout << "Item updated successfully!" << std::endl;

								modifyButton.setColor(sf::Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(sf::Color::White);
								submitButton.setPosition({ 210,300 });

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
								std::cerr  << "Failed to update item!" << std::endl;
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);						
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT3 = true;

						clock.restart();
					}
				}
			}
		}

		if (transactionButton.isPressed())
		{
			clickItem = false;
			clickAisle = false;
			clickSection = false;
			clickSupplier = false;
			clickTransaction = true;
			clickSearch = false;
			clickGO = false;

			currBackground.setTextureRect(backgrounds[5]);

			t1Box.setValid();
			t2Box.setValid();
			t3Box.setValid();
			t4Box.setValid();
			t5Box.setValid();
			t6Box.setValid();

			submitButton.setPosition({ 210,400 });
			submitButton.setColor(sf::Color::White);

			modifyButton.setPosition({ 0,0 });
			modifyButton.setColor(sf::Color::Transparent);

			transactionButton.setPressedOff();
		}

		if (clickTransaction)
		{
			t1Box.drawTextBox(ImVec2(247,88), "##transactionT1InputWindow", "##transactionT1Input");
			t2Box.drawTextBox(ImVec2(247,136), "##transactionT2InputWindow", "##transactionT2Input");
			t3Box.drawTextBox(ImVec2(247,184), "##transactionT3InputWindow", "##transactionT3Input");
			t4Box.drawTextBox(ImVec2(247,232), "##transactionT4InputWindow", "##transactionT4Input");
			t5Box.drawTextBox(ImVec2(247,280), "##transactionT5InputWindow", "##transactionT5Input");
			t6Box.drawTextBox(ImVec2(247,328), "##transactionT6InputWindow", "##transactionT6Input");

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
							const std::string& selectedTransactionId = transactions[selectedRow].transaction_id5;

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring deleteQuery = L"DELETE FROM transaction WHERE transaction_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)deleteQuery.c_str(), SQL_NTS);

							std::wstring transaction_id_wstr(selectedTransactionId.begin(), selectedTransactionId.end());

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_id_wstr.c_str(), 0, NULL);

							SQLRETURN retSQL = SQLExecute(handleSQL);

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								std::cout << "Delete Successful" << std::endl;

								transactions.erase(transactions.begin() + selectedRow);
								selectedRow = -1;

								submitButton.setPosition({ 210,400 });
								submitButton.setColor(sf::Color::White);

								modifyButton.setPosition({ 0,0 });
								modifyButton.setColor(sf::Color::Transparent);
							}
							else
							{
								std::cerr  << "Delete failed" << std::endl;
							}
						}
					}

					//TODO fix this idk what it even does
					/*
					ImGui::SameLine();
					if (ImGui::Button("Modify"))
					{
						if (selectedRow >= 0 && selectedRow < transactions.size())
						{
							submitButton.setColor(sf::Color::Transparent);
							submitButton.setPosition({ 0,0 });

							modifyButton.setColor(sf::Color::White);
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
					*/
				}

				ImGui::EndChild();
			}

			ImGui::End();
			ImGui::PopStyleColor(2);
		}

		if (clickTransaction)
		{
			if (mouseDetector.isOn(submitButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						if (t4Box.getInput() == "")
						{
							std::cerr  << "Empty t4" << std::endl;
							notNull = false;

							t4Box.setInvalid();
						}
						else
						{
							t4Box.setValid();
						}

						if (t5Box.getInput() == "")
						{
							std::cerr  << "Empty t5" << std::endl;
							notNull = false;

							t5Box.setInvalid();
						}
						else
						{
							t5Box.setInvalid();
						}

						if (t6Box.getInput() == "")
						{
							std::cerr  << "Empty t6" << std::endl;
							notNull = false;

							t6Box.setInvalid();
						}
						else
						{
							t6Box.setValid();
						}

						try
						{
							std::string t3InTEST(t3Box.getInput());

							std::stof(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t2Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						try
						{
							std::string t4InTEST(t4Box.getInput());

							std::stof(t4InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t4 number" << std::endl;
							valNums = false;
							valT4 = false;

							t4Box.setInvalid();
						}
						if (valT4)
						{
							t4Box.setValid();
						}

						try
						{
							std::string t5InTEST(t5Box.getInput());

							std::stof(t5InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t6 number" << std::endl;
							valNums = false;
							valT5 = false;

							t5Box.setInvalid();
						}
						if (valT5)
						{
							t5Box.setInvalid();
						}

						if (notNull && valNums)
						{
							std::string transaction_idStr(t1Box.getInput());
							std::wstring transaction_id(transaction_idStr.begin(), transaction_idStr.end());

							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
							SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT transaction_id FROM transaction WHERE transaction_id = ?", SQL_NTS);
							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)transaction_id.c_str(), 0, nullptr);
							retSQL = SQLExecute(handleSQL);

							if (retSQL == SQL_SUCCESS || retSQL == SQL_SUCCESS_WITH_INFO)
							{
								if (SQLFetch(handleSQL) == SQL_SUCCESS)
								{
									std::cout << "Transaction ID already exists" << std::endl;
									primaryKeyIsVal = false;

									t1Box.setInvalid();

									int primaryKeyError = MessageBoxW(
										nullptr,
										L"Transaction I.D. is already in use.\n\n"
										L"Please try again.",
										L"Duplicate Primary Key!",
										MB_OK | MB_ICONWARNING
									);
								}
								else
								{
									primaryKeyIsVal = true;

									t1Box.setValid();
								}
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

							if (primaryKeyIsVal)
							{

								std::string item_idStr(t2Box.getInput());
								std::wstring item_id(item_idStr.begin(), item_idStr.end());

								SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
								SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item_id FROM item WHERE item_id = ?", SQL_NTS);
								SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
								retSQL = SQLExecute(handleSQL);

								if (retSQL == SQL_SUCCESS || retSQL == SQL_SUCCESS_WITH_INFO)
								{
									if (SQLFetch(handleSQL) != SQL_SUCCESS)
									{
										std::cout << "Item ID does exists!" << std::endl;
										valItemID = false;

										t2Box.setInvalid();

										int primaryKeyError = MessageBoxW(
											nullptr,
											L"Item I.D. does not exist.\n\n"
											L"Please try again.",
											L"Foreign Key Constraint",
											MB_OK | MB_ICONWARNING
										);
									}
									else
									{
										valItemID = true;

										t2Box.setValid();
									}
								}

								SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);

								if (valItemID)
								{
									float item_price = atof(t3Box.getInput());
									float tax_amount = atof(t4Box.getInput());
									float transaction_total = atof(t5Box.getInput());
									std::string transaction_dateStr(t6Box.getInput());
									std::wstring transaction_date(transaction_dateStr.begin(), transaction_dateStr.end());

									std::wstring insertQuery = L"INSERT INTO transaction (transaction_id, item_id, item_price, tax_amount, transaction_total, transaction_date) VALUES (?,?,?,?,?,?)";

									SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

									SQLPrepare(handleSQL, (SQLWCHAR*)insertQuery.c_str(), SQL_NTS);

									SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_id.c_str(), 0, NULL);
									SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
									SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &item_price, 0, NULL);
									SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &tax_amount, 0, NULL);
									SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_FLOAT, 0, 0, &transaction_total, 0, NULL);
									SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_date.c_str(), 0, NULL);

									retSQL = SQLExecute(handleSQL);

									if (SQL_SUCCEEDED(retSQL))
									{
										std::cout << "Insert successful!" << std::endl;

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
										std::cerr  << "Insert failed!" << std::endl;
									}

									SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
								}
							}
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT3 = true;
						valT4 = true;
						valT5 = true;

						clock.restart();
					}
				}
			}

			if (mouseDetector.isOn(modifyButton, window))
			{
				if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
				{
					if (clock.getElapsedTime().asSeconds() >= 0.3)
					{
						if (t1Box.getInput() == "")
						{
							std::cerr  << "Empty t1" << std::endl;
							notNull = false;

							t1Box.setInvalid();
						}
						else
						{
							t1Box.setValid();
						}

						if (t2Box.getInput() == "")
						{
							std::cerr  << "Empty t2" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t2Box.setValid();
						}

						if (t3Box.getInput() == "")
						{
							std::cerr  << "Empty t3" << std::endl;
							notNull = false;

							t2Box.setInvalid();
						}
						else
						{
							t3Box.setValid();
						}

						if (t4Box.getInput() == "")
						{
							std::cerr  << "Empty t4" << std::endl;
							notNull = false;

							t4Box.setInvalid();
						}
						else
						{
							t4Box.setValid();
						}

						if (t5Box.getInput() == "")
						{
							std::cerr  << "Empty t5" << std::endl;
							notNull = false;

							t5Box.setInvalid();
						}
						else
						{
							t5Box.setInvalid();
						}

						if (t6Box.getInput() == "")
						{
							std::cerr  << "Empty t6" << std::endl;
							notNull = false;

							t6Box.setInvalid();
						}
						else
						{
							t6Box.setValid();
						}

						try
						{
							std::string t3InTEST(t3Box.getInput());

							std::stof(t3InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t3 number" << std::endl;
							valNums = false;
							valT3 = false;

							t2Box.setInvalid();
						}
						if (valT3)
						{
							t3Box.setValid();
						}

						try
						{
							std::string t4InTEST(t4Box.getInput());

							std::stof(t4InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t4 number" << std::endl;
							valNums = false;
							valT4 = false;

							t4Box.setInvalid();
						}
						if (valT4)
						{
							t4Box.setValid();
						}

						try
						{
							std::string t5InTEST(t5Box.getInput());

							std::stof(t5InTEST);
						}
						catch (const std::exception& e)
						{
							std::cerr  << "invalid t6 number" << std::endl;
							valNums = false;
							valT5 = false;

							t5Box.setInvalid();
						}
						if (valT5)
						{
							t5Box.setInvalid();
						}

						if (notNull && valNums)
						{
							SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);

							std::wstring original_transaction_idW(original_transaction_id5.begin(), original_transaction_id5.end());
							std::string transaction_idStr(t1Box.getInput());
							std::wstring transaction_id(transaction_idStr.begin(), transaction_idStr.end());
							std::string item_idStr(t2Box.getInput());
							std::wstring item_id(item_idStr.begin(), item_idStr.end());
							float item_price = atof(t3Box.getInput());
							float tax_amount = atof(t4Box.getInput());
							float transaction_total = atof(t5Box.getInput());
							std::string transaction_dateStr(t6Box.getInput());
							std::wstring transaction_date(transaction_dateStr.begin(), transaction_dateStr.end());

							std::wstring updateQuery = L"UPDATE transaction SET transaction_id = ?, item_id = ?, item_price = ?, tax_amount = ?, transaction_total = ?, transaction_date = ? WHERE transaction_id = ?";

							SQLPrepare(handleSQL, (SQLWCHAR*)updateQuery.c_str(), SQL_NTS);

							SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 2, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)item_id.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 3, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &item_price, 0, NULL);
							SQLBindParameter(handleSQL, 4, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &tax_amount, 0, NULL);
							SQLBindParameter(handleSQL, 5, SQL_PARAM_INPUT, SQL_C_FLOAT, SQL_REAL, 0, 0, &transaction_total, 0, NULL);
							SQLBindParameter(handleSQL, 6, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)transaction_date.c_str(), 0, NULL);
							SQLBindParameter(handleSQL, 7, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 0, 0, (SQLPOINTER)original_transaction_idW.c_str(), 0, NULL);


							retSQL = SQLExecute(handleSQL);

							if (SQL_SUCCEEDED(retSQL))
							{
								std::cout << "Item updated successfully!" << std::endl;

								modifyButton.setColor(sf::Color::Transparent);
								modifyButton.setPosition({ 0,0 });

								submitButton.setColor(sf::Color::White);
								submitButton.setPosition({ 210,400 });

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
								std::cerr << "Failed to update item!" << std::endl;
							}

							SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
						}

						else
						{
							int error = MessageBoxW(
								nullptr,
								L"One or more inputs are either invalid or empty.\n"
								L"Invalid/Empty inputs are highlighted red.\n\n"
								L"Please try again.",
								L"Invalid/Empty Inputs!",
								MB_OK | MB_ICONWARNING
							);
						}
						notNull = true;
						valNums = true;
						valT3 = true;
						valT4 = true;
						valT5 = true;

						clock.restart();
					}
				}
			}
		}

		if (mouseDetector.isOn(searchButton, window))
		{
			if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
			{
				clickItem = false;
				clickAisle = false;
				clickSection = false;
				clickSupplier = false;
				clickTransaction = false;
				clickSearch = true;
				clickGO = false;
				
				currBackground.setTextureRect(backgrounds[0]);

				t1Box.setValid();
				t2Box.setValid();
				t3Box.setValid();
				t4Box.setValid();
				t5Box.setValid();
				t6Box.setValid();

				submitButton.setColor(sf::Color::Transparent);
				modifyButton.setColor(sf::Color::Transparent);
			}
		}

		if (clickSearch)
		{
			const char* options[] = { "Item I.D.", "Aisle No.", "Section I.D.", "Supplier I.D.", "Transaction I.D.", "Date Sold"};
			static int current_option = 0;

			ImGui::SetNextWindowPos(ImVec2(15, 63));//x-offset:8	y-offset:+11
			ImGui::SetNextWindowSize(ImVec2(150, 100)); //x-offset:81
			ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
			ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
			ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

			ImGui::Begin("Search By", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

			ImGui::SetNextItemWidth(135.f);
			ImGui::Combo("##hidden", &current_option, options, IM_ARRAYSIZE(options));

			ImGui::SetNextItemWidth(135.f);
			ImGui::InputText("##searcht1Input", t1Box.getInput(), sizeof(t1Box.getInput()));

			if (ImGui::Button("Go"))
			{
				clickGO = true;

				// item id option
				if (current_option == 0)
				{
					showSearchItem = true;
					showSearchAisle = true;
					showSearchSection = true;
					showSearchSupplier = true;
					showSearchTransaction = true;

					if (t1Box.getInput() == "")
					{
						notNull = false;
						clickGO = false;

						int nullerror = MessageBoxW(
							nullptr,
							L"Input is empty.\n\n"
							L"Please try again.",
							L"Empty Input!",
							MB_OK | MB_ICONWARNING
						);
					}

					if (notNull)
					{
						std::string item_idStr(t1Box.getInput());
						std::wstring item_id(item_idStr.begin(), item_idStr.end());

						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item.* FROM item WHERE item_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);
					
						if (retSQL == SQL_SUCCESS)
						{							
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_item_id1, sizeof(search_item_id1), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_name1, sizeof(search_item_name1), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no1, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_section_id1, sizeof(search_section_id1), nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_item_price1, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_SLONG, &search_no_of_items1, 0, nullptr);

							searchitems.clear();
							bool found = false;

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								found = true;
								searchitems.push_back({
									search_item_id1,
									search_item_name1,
									search_aisle_no1,
									search_section_id1,
									search_item_price1,
									search_no_of_items1
									});
							}

							if (!found)
							{
								clickGO = false;
								
								int notfounderror = MessageBoxW(
									nullptr,
									L"Item I.D. does not exist.\n\n"
									L"Please try again.",
									L"Invalid Item I.D.!",
									MB_OK | MB_ICONWARNING
								);
							}
						}

						else
						{
							std::cout << "item failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT aisle.* FROM aisle JOIN item ON item.aisle_no = aisle.aisle_no WHERE item_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_SLONG, &search_aisle_no2, 0, nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_SLONG, &search_no_of_sections2, 0, nullptr);

							searchaisles.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchaisles.push_back({
									search_aisle_no2,
									search_no_of_sections2
									});
							}
						}

						else
						{
							std::cout << "aisle failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT section.* FROM section JOIN item ON item.section_id = section.section_id WHERE item_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_section_id3, sizeof(search_section_id3), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_section_name3, sizeof(search_section_name3), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no3, 0, nullptr);

							searchsections.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchsections.push_back({
									search_section_id3,
									search_section_name3,
									search_aisle_no3
									});
							}
						}

						else
						{
							std::cout << "section failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT supplier.* FROM supplier JOIN item ON item.item_id = supplier.item_id WHERE item.item_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_supplier_id4, sizeof(search_supplier_id4), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_id4, sizeof(search_item_id4), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &search_item_cost4, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_supplier_name4, sizeof(search_supplier_name4), nullptr);

							searchsuppliers.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchsuppliers.push_back({
									search_supplier_id4,
									search_item_id4,
									search_item_cost4,
									search_supplier_name4
									});
							}
						}

						else
						{
							std::cout << "supplier failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT transaction.* FROM transaction JOIN item ON item.item_id = transaction.item_id WHERE item.item_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)item_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_transaction_id5, sizeof(search_transaction_id5), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_id5, sizeof(search_item_id5), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &search_item_price5, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_FLOAT, &search_tax_amount5, 0, nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_transaction_total5, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_CHAR, search_transaction_date5, sizeof(search_transaction_date5), nullptr);

							searchtransactions.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchtransactions.push_back({
										search_transaction_id5,
										search_item_id5,
										search_item_price5,
										search_tax_amount5,
										search_transaction_total5,
										search_transaction_date5
									});
							}
						}

						else
						{
							std::cout << "transaction failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
					}

					notNull = true;	
				}

				if (current_option == 1)
				{
					showSearchItem = true;
					showSearchAisle = true;
					showSearchSection = true;
					showSearchSupplier = false;
					showSearchTransaction = false;

					if (t1Box.getInput() == "")
					{
						notNull = false;
						clickGO = false;

						int nullerror = MessageBoxW(
							nullptr,
							L"Input is empty.\n\n"
							L"Please try again.",
							L"Empty Input!",
							MB_OK | MB_ICONWARNING
						);
					}


					if (notNull)
					{
						std::string aisle_noStr(t1Box.getInput());
						std::wstring aisle_no(aisle_noStr.begin(), aisle_noStr.end());

						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item.* FROM item JOIN aisle ON aisle.aisle_no = item.aisle_no WHERE aisle.aisle_no = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)aisle_no.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_item_id1, sizeof(search_item_id1), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_name1, sizeof(search_item_name1), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no1, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_section_id1, sizeof(search_section_id1), nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_item_price1, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_SLONG, &search_no_of_items1, 0, nullptr);

							searchitems.clear();
							bool found = false;

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								found = true;
								searchitems.push_back({
									search_item_id1,
									search_item_name1,
									search_aisle_no1,
									search_section_id1,
									search_item_price1,
									search_no_of_items1
									});
							}

							if (!found)
							{
								clickGO = false;

								int notfounderror = MessageBoxW(
									nullptr,
									L"Aisle No. does not exist.\n\n"
									L"Please try again.",
									L"Invalid Aisle No.!",
									MB_OK | MB_ICONWARNING
								);
							}
						}

						else
						{
							std::cout << "item failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT aisle.* FROM aisle WHERE aisle_no = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)aisle_no.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_SLONG, &search_aisle_no2, 0, nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_SLONG, &search_no_of_sections2, 0, nullptr);

							searchaisles.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchaisles.push_back({
									search_aisle_no2,
									search_no_of_sections2
									});
							}
						}

						else
						{
							std::cout << "aisle failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT section.* FROM section JOIN aisle ON aisle.aisle_no = section.aisle_no WHERE aisle.aisle_no = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)aisle_no.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_section_id3, sizeof(search_section_id3), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_section_name3, sizeof(search_section_name3), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no3, 0, nullptr);

							searchsections.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchsections.push_back({
									search_section_id3,
									search_section_name3,
									search_aisle_no3
									});
							}
						}

						else
						{
							std::cout << "section failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
					}

					notNull = true;
				}

				if (current_option == 2)
				{
					showSearchItem = true;
					showSearchAisle = true;
					showSearchSection = true;
					showSearchSupplier = false;
					showSearchTransaction = false;


					if (t1Box.getInput() == "")
					{
						notNull = false;
						clickGO = false;

						int nullerror = MessageBoxW(
							nullptr,
							L"Input is empty.\n\n"
							L"Please try again.",
							L"Empty Input!",
							MB_OK | MB_ICONWARNING
						);
					}

					if (notNull)
					{
						std::string section_idStr(t1Box.getInput());
						std::wstring section_id(section_idStr.begin(), section_idStr.end());

						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item.* FROM item JOIN section ON section.section_id = item.section_id WHERE section.section_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)section_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_item_id1, sizeof(search_item_id1), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_name1, sizeof(search_item_name1), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no1, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_section_id1, sizeof(search_section_id1), nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_item_price1, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_SLONG, &search_no_of_items1, 0, nullptr);

							searchitems.clear();
							bool found = false;

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								found = true;
								searchitems.push_back({
									search_item_id1,
									search_item_name1,
									search_aisle_no1,
									search_section_id1,
									search_item_price1,
									search_no_of_items1
									});
							}

							if (!found)
							{
								clickGO = false;

								int notfounderror = MessageBoxW(
									nullptr,
									L"Section I.D. does not exist.\n\n"
									L"Please try again.",
									L"Invalid Section I.D.!",
									MB_OK | MB_ICONWARNING
								);
							}
						}

						else
						{
							std::cout << "item failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT aisle.* FROM aisle JOIN section ON section.aisle_no = aisle.aisle_no WHERE section_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)section_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_SLONG, &search_aisle_no2, 0, nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_SLONG, &search_no_of_sections2, 0, nullptr);

							searchaisles.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchaisles.push_back({
									search_aisle_no2,
									search_no_of_sections2
									});
							}
						}

						else
						{
							std::cout << "aisle failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT section.* FROM section WHERE section_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)section_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_section_id3, sizeof(search_section_id3), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_section_name3, sizeof(search_section_name3), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no3, 0, nullptr);

							searchsections.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchsections.push_back({
									search_section_id3,
									search_section_name3,
									search_aisle_no3
									});
							}
						}

						else
						{
							std::cout << "section failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
					}

					notNull = true;	
				}

				if (current_option == 3)
				{
					showSearchItem = true;
					showSearchAisle = false;
					showSearchSection = false;
					showSearchSupplier = true;
					showSearchTransaction = false;

					if (t1Box.getInput() == "")
					{
						notNull = false;
						clickGO = false;

						int nullerror = MessageBoxW(
							nullptr,
							L"Input is empty.\n\n"
							L"Please try again.",
							L"Empty Input!",
							MB_OK | MB_ICONWARNING
						);
					}

					if (notNull)
					{
						std::string supplier_idStr(t1Box.getInput());
						std::wstring supplier_id(supplier_idStr.begin(), supplier_idStr.end());

						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item.* FROM item JOIN supplier ON supplier.item_id = item.item_id WHERE supplier.supplier_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)supplier_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_item_id1, sizeof(search_item_id1), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_name1, sizeof(search_item_name1), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no1, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_section_id1, sizeof(search_section_id1), nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_item_price1, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_SLONG, &search_no_of_items1, 0, nullptr);

							searchitems.clear();
							bool found = false;

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								found = true;
								searchitems.push_back({
									search_item_id1,
									search_item_name1,
									search_aisle_no1,
									search_section_id1,
									search_item_price1,
									search_no_of_items1
									});
							}

							if (!found)
							{
								clickGO = false;

								int notfounderror = MessageBoxW(
									nullptr,
									L"Supplier I.D. does not exist.\n\n"
									L"Please try again.",
									L"Invalid Supplier I.D.!",
									MB_OK | MB_ICONWARNING
								);
							}
						}

						else
						{
							std::cout << "item failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT supplier.* FROM supplier WHERE supplier_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)supplier_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_supplier_id4, sizeof(search_supplier_id4), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_id4, sizeof(search_item_id4), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &search_item_cost4, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_supplier_name4, sizeof(search_supplier_name4), nullptr);

							searchsuppliers.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchsuppliers.push_back({
									search_supplier_id4,
									search_item_id4,
									search_item_cost4,
									search_supplier_name4
									});
							}
						}

						else
						{
							std::cout << "supplier failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
					}

					notNull = true;	
				}

				if (current_option == 4)
				{
					showSearchItem = true;
					showSearchAisle = false;
					showSearchSection = false;
					showSearchSupplier = false;
					showSearchTransaction = true;

					if (t1Box.getInput() == "")
					{
						notNull = false;
						clickGO = false;

						int nullerror = MessageBoxW(
							nullptr,
							L"Input is empty.\n\n"
							L"Please try again.",
							L"Empty Input!",
							MB_OK | MB_ICONWARNING
						);
					}

					if (notNull)
					{
						std::string transaction_idStr(t1Box.getInput());
						std::wstring transaction_id(transaction_idStr.begin(), transaction_idStr.end());

						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item.* FROM item JOIN transaction ON transaction.item_id = item.item_id WHERE transaction_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)transaction_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_item_id1, sizeof(search_item_id1), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_name1, sizeof(search_item_name1), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no1, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_section_id1, sizeof(search_section_id1), nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_item_price1, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_SLONG, &search_no_of_items1, 0, nullptr);

							searchitems.clear();
							bool found = false;

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								found = true;
								searchitems.push_back({
									search_item_id1,
									search_item_name1,
									search_aisle_no1,
									search_section_id1,
									search_item_price1,
									search_no_of_items1
									});
							}

							if (!found)
							{
								clickGO = false;

								int notfounderror = MessageBoxW(
									nullptr,
									L"Transaction I.D. does not exist.\n\n"
									L"Please try again.",
									L"Invalid Transaction I.D.!",
									MB_OK | MB_ICONWARNING
								);
							}
						}

						else
						{
							std::cout << "item failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT transaction.* FROM transaction WHERE transaction_id = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)transaction_id.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_transaction_id5, sizeof(search_transaction_id5), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_id5, sizeof(search_item_id5), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &search_item_price5, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_FLOAT, &search_tax_amount5, 0, nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_transaction_total5, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_CHAR, search_transaction_date5, sizeof(search_transaction_date5), nullptr);

							searchtransactions.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchtransactions.push_back({
										search_transaction_id5,
										search_item_id5,
										search_item_price5,
										search_tax_amount5,
										search_transaction_total5,
										search_transaction_date5
									});
							}
						}

						else
						{
							std::cout << "transaction failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
					}

					notNull = true;
				}

				if (current_option == 5)
				{
					showSearchItem = true;
					showSearchAisle = false;
					showSearchSection = false;
					showSearchSupplier = false;
					showSearchTransaction = true;

					if (t1Box.getInput() == "")
					{
						notNull = false;
						clickGO = false;

						int nullerror = MessageBoxW(
							nullptr,
							L"Input is empty.\n\n"
							L"Please try again.",
							L"Empty Input!",
							MB_OK | MB_ICONWARNING
						);
					}

					if (notNull)
					{
						std::string transaction_dateStr(t1Box.getInput());
						std::wstring transaction_date(transaction_dateStr.begin(), transaction_dateStr.end());

						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT item.* FROM item JOIN transaction ON transaction.item_id = item.item_id WHERE transaction_date = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)transaction_date.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_item_id1, sizeof(search_item_id1), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_name1, sizeof(search_item_name1), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_SLONG, &search_aisle_no1, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_CHAR, search_section_id1, sizeof(search_section_id1), nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_item_price1, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_SLONG, &search_no_of_items1, 0, nullptr);

							searchitems.clear();
							bool found = false;

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								found = true;
								searchitems.push_back({
									search_item_id1,
									search_item_name1,
									search_aisle_no1,
									search_section_id1,
									search_item_price1,
									search_no_of_items1
									});
							}

							if (!found)
							{
								clickGO = false;

								int notfounderror = MessageBoxW(
									nullptr,
									L"Transaction Date does not exist.\n\n"
									L"Please try again.",
									L"Invalid Transaction Date!",
									MB_OK | MB_ICONWARNING
								);
							}
						}

						else
						{
							std::cout << "item failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);


						SQLAllocHandle(SQL_HANDLE_STMT, dbconSQL, &handleSQL);
						SQLPrepareW(handleSQL, (SQLWCHAR*)L"SELECT transaction.* FROM transaction WHERE transaction_date = ?", SQL_NTS);
						SQLBindParameter(handleSQL, 1, SQL_PARAM_INPUT, SQL_C_WCHAR, SQL_WVARCHAR, 100, 0, (SQLPOINTER)transaction_date.c_str(), 0, nullptr);
						retSQL = SQLExecute(handleSQL);

						if (retSQL == SQL_SUCCESS)
						{
							SQLBindCol(handleSQL, 1, SQL_C_CHAR, search_transaction_id5, sizeof(search_transaction_id5), nullptr);
							SQLBindCol(handleSQL, 2, SQL_C_CHAR, search_item_id5, sizeof(search_item_id5), nullptr);
							SQLBindCol(handleSQL, 3, SQL_C_FLOAT, &search_item_price5, 0, nullptr);
							SQLBindCol(handleSQL, 4, SQL_C_FLOAT, &search_tax_amount5, 0, nullptr);
							SQLBindCol(handleSQL, 5, SQL_C_FLOAT, &search_transaction_total5, 0, nullptr);
							SQLBindCol(handleSQL, 6, SQL_C_CHAR, search_transaction_date5, sizeof(search_transaction_date5), nullptr);

							searchtransactions.clear();

							while (SQLFetch(handleSQL) == SQL_SUCCESS)
							{
								searchtransactions.push_back({
										search_transaction_id5,
										search_item_id5,
										search_item_price5,
										search_tax_amount5,
										search_transaction_total5,
										search_transaction_date5
									});
							}
						}

						else
						{
							std::cout << "transaction failed" << std::endl;

							clickGO = false;
						}

						SQLFreeHandle(SQL_HANDLE_STMT, handleSQL);
					}

					notNull = true;
				}
			}
		
			ImGui::PopStyleColor(2);
			ImGui::End();
		}

		if (clickGO)
		{
			int selectedRow = -1;
			int currentRow = 0;

			if (showSearchItem)
			{
				ImGui::SetNextWindowPos(ImVec2(200, 63));
				ImGui::SetNextWindowSize(ImVec2(600, 250));
				ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
				ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

				if (ImGui::Begin("Items", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
				{
					ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

					if (ImGui::BeginTable("##SearchItemsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
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

							sort(searchitems.begin(), searchitems.end(), [&](const SearchItem& a, const SearchItem& b)
								{
									switch (columnIndex)
									{
									case 0: return ascending ? a.search_item_id1 < b.search_item_id1 : a.search_item_id1 > b.search_item_id1;
									case 1: return ascending ? a.search_item_name1 < b.search_item_name1 : a.search_item_name1 > b.search_item_name1;
									case 2: return ascending ? a.search_aisle_no1 < b.search_aisle_no1 : a.search_aisle_no1 > b.search_aisle_no1;
									case 3: return ascending ? a.search_section_id1 < b.search_section_id1 : a.search_section_id1 > b.search_section_id1;
									case 4: return ascending ? a.search_item_price1 < b.search_item_price1 : a.search_item_price1 > b.search_item_price1;
									case 5: return ascending ? a.search_no_of_items1 < b.search_no_of_items1 : a.search_no_of_items1 > b.search_no_of_items1;
									default: return false;
									}
								});

							sortSpecs->SpecsDirty = false;
						}

						for (const auto& searchItem : searchitems)
						{
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							bool isSelected = (currentRow == selectedRow);

							if (ImGui::Selectable(searchItem.search_item_id1.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap))
							{
								selectedRow = currentRow;
							}

							ImGui::TableSetColumnIndex(1); ImGui::Text("%s", searchItem.search_item_name1.c_str());
							ImGui::TableSetColumnIndex(2); ImGui::Text("%d", searchItem.search_aisle_no1);
							ImGui::TableSetColumnIndex(3); ImGui::Text("%s", searchItem.search_section_id1.c_str());
							ImGui::TableSetColumnIndex(4); ImGui::Text("%.2f", searchItem.search_item_price1);
							ImGui::TableSetColumnIndex(5); ImGui::Text("%d", searchItem.search_no_of_items1);

							currentRow++;
						}

						ImGui::EndTable();
					}
				}

				ImGui::PopStyleColor(2);
				ImGui::EndChild();
				ImGui::End();
			}

			if (showSearchAisle)
			{
				ImGui::SetNextWindowPos(ImVec2(825, 63));
				ImGui::SetNextWindowSize(ImVec2(400, 250));
				ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
				ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

				if (ImGui::Begin("Aisles", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
				{
					ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

					if (ImGui::BeginTable("##SearchAislesTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
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

							sort(searchaisles.begin(), searchaisles.end(), [&](const SearchAisle& a, const SearchAisle& b)
								{
									switch (columnIndex)
									{
									case 0: return ascending ? a.search_aisle_no2 < b.search_aisle_no2 : a.search_aisle_no2 > b.search_aisle_no2;
									case 1: return ascending ? a.search_no_of_sections2 < b.search_no_of_sections2 : a.search_no_of_sections2 > b.search_no_of_sections2;
									default: return false;
									}
								});

							sortSpecs->SpecsDirty = false;
						}

						for (const auto& searchaisle : searchaisles)
						{
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							bool isSelected = (currentRow == selectedRow);
							std::string aisleStr = std::to_string(searchaisle.search_aisle_no2);
							if (ImGui::Selectable(aisleStr.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
								selectedRow = currentRow;
							}

							ImGui::TableSetColumnIndex(1); ImGui::Text("%d", searchaisle.search_no_of_sections2);

							currentRow++;
						}

						ImGui::EndTable();
					}
				}

				ImGui::PopStyleColor(2);
				ImGui::EndChild();
				ImGui::End();
			}

			if (showSearchSection)
			{
				ImGui::SetNextWindowPos(ImVec2(15, 350));
				ImGui::SetNextWindowSize(ImVec2(420, 150));
				ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
				ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

				if (ImGui::Begin("Sections", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
				{
					ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

					if (ImGui::BeginTable("##SearchSectionsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
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

							sort(searchsections.begin(), searchsections.end(), [&](const SearchSection& a, const SearchSection& b)
								{
									switch (columnIndex)
									{
									case 0: return ascending ? a.search_section_id3 < b.search_section_id3 : a.search_section_id3 > b.search_section_id3;
									case 1: return ascending ? a.search_section_name3 < b.search_section_name3 : a.search_section_name3 > b.search_section_name3;
									case 2: return ascending ? a.search_aisle_no3 < b.search_aisle_no3 : a.search_aisle_no3 > b.search_aisle_no3;
									default: return false;
									}
								});

							sortSpecs->SpecsDirty = false;
						}

						for (const auto& searchsection : searchsections)
						{
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							bool isSelected = (currentRow == selectedRow);

							if (ImGui::Selectable(searchsection.search_section_id3.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
								selectedRow = currentRow;
							}

							ImGui::TableSetColumnIndex(1); ImGui::Text("%s", searchsection.search_section_name3.c_str());
							ImGui::TableSetColumnIndex(2); ImGui::Text("%d", searchsection.search_aisle_no3);

							currentRow++;
						}

						ImGui::EndTable();
					}
				}

				ImGui::PopStyleColor(2);
				ImGui::EndChild();
				ImGui::End();
			}

			if (showSearchSupplier)
			{
				ImGui::SetNextWindowPos(ImVec2(15, 510));
				ImGui::SetNextWindowSize(ImVec2(420, 150));
				ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
				ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

				if (ImGui::Begin("Suppliers", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
				{
					ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

					if (ImGui::BeginTable("##SearchSuppliersTable", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
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

							sort(searchsuppliers.begin(), searchsuppliers.end(), [&](const SearchSupplier& a, const SearchSupplier& b)
								{
									switch (columnIndex)
									{
									case 0: return ascending ? a.search_supplier_id4 < b.search_supplier_id4 : a.search_supplier_id4 > b.search_supplier_id4;
									case 1: return ascending ? a.search_item_id4 < b.search_item_id4 : a.search_item_id4 > b.search_item_id4;
									case 2: return ascending ? a.search_item_cost4 < b.search_item_cost4 : a.search_item_cost4 > b.search_item_cost4;
									case 3: return ascending ? a.search_supplier_name4 < b.search_supplier_name4 : a.search_supplier_name4 > b.search_supplier_name4;
									default: return false;
									}
								});

							sortSpecs->SpecsDirty = false;
						}

						for (const auto& searchsupplier : searchsuppliers)
						{
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							bool isSelected = (currentRow == selectedRow);

							if (ImGui::Selectable(searchsupplier.search_supplier_id4.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
								selectedRow = currentRow;
							}

							ImGui::TableSetColumnIndex(1); ImGui::Text("%s", searchsupplier.search_item_id4.c_str());
							ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", searchsupplier.search_item_cost4);
							ImGui::TableSetColumnIndex(3); ImGui::Text("%s", searchsupplier.search_supplier_name4.c_str());

							currentRow++;
						}

						ImGui::EndTable();
					}
				}

				ImGui::PopStyleColor(2);
				ImGui::EndChild();
				ImGui::End();
			}

			if (showSearchTransaction)
			{
				ImGui::SetNextWindowPos(ImVec2(470, 350));
				ImGui::SetNextWindowSize(ImVec2(750, 310));
				ImVec4 headerColor = ImVec4(62.0f / 255.0f, 127.0f / 255.0f, 8.0f / 255.0f, 1.0f);
				ImGui::PushStyleColor(ImGuiCol_TitleBg, headerColor);
				ImGui::PushStyleColor(ImGuiCol_TitleBgActive, headerColor);

				if (ImGui::Begin("Transactions", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse))
				{
					ImGui::BeginChild("TableScrollable", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);

					if (ImGui::BeginTable("##SearchTransactionsTable", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_Sortable))
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

							sort(searchtransactions.begin(), searchtransactions.end(), [&](const SearchTransaction& a, const SearchTransaction& b)
								{
									switch (columnIndex)
									{
									case 0: return ascending ? a.search_transaction_id5 < b.search_transaction_id5 : a.search_transaction_id5 > b.search_transaction_id5;
									case 1: return ascending ? a.search_item_id5 < b.search_item_id5 : a.search_item_id5 > b.search_item_id5;
									case 2: return ascending ? a.search_item_price5 < b.search_item_price5 : a.search_item_price5 > b.search_item_price5;
									case 3: return ascending ? a.search_tax_amount5 < b.search_tax_amount5 : a.search_tax_amount5 > b.search_tax_amount5;
									case 4: return ascending ? a.search_transaction_total5 < b.search_transaction_total5 : a.search_transaction_total5 > b.search_transaction_total5;
									case 5: return ascending ? a.search_transaction_date5 < b.search_transaction_date5 : a.search_transaction_date5 > b.search_transaction_date5;
									default: return false;
									}
								});

							sortSpecs->SpecsDirty = false;
						}

						for (const auto& searchtransaction : searchtransactions)
						{
							ImGui::TableNextRow();

							ImGui::TableSetColumnIndex(0);
							bool isSelected = (currentRow == selectedRow);

							if (ImGui::Selectable(searchtransaction.search_transaction_id5.c_str(), isSelected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
								selectedRow = currentRow;
							}

							ImGui::TableSetColumnIndex(1); ImGui::Text("%s", searchtransaction.search_item_id5.c_str());
							ImGui::TableSetColumnIndex(2); ImGui::Text("%.2f", searchtransaction.search_item_price5);
							ImGui::TableSetColumnIndex(3); ImGui::Text("%.2f", searchtransaction.search_tax_amount5);
							ImGui::TableSetColumnIndex(4); ImGui::Text("%.2f", searchtransaction.search_transaction_total5);
							ImGui::TableSetColumnIndex(5); ImGui::Text("%s", searchtransaction.search_transaction_date5.c_str());

							currentRow++;
						}

						ImGui::EndTable();
					}
				}

				ImGui::PopStyleColor(2);
				ImGui::EndChild();
				ImGui::End();
			}
		}

		window.clear();
		window.draw(currBackground);
		window.draw(submitButton);
		window.draw(modifyButton);
		ImGui::SFML::Render(window);
		window.display();
	}

	ImGui::SFML::Shutdown();
}