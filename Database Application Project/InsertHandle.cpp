#include "InsertHandle.h"

using namespace std;

void InsertHandle::Insert(std::string values, SQLHDBC dbCon, SQLHSTMT handle, SQLRETURN rtrn)
{
	wstring wInput(values.begin(), values.end());
	wstring insertQuery = L"INSERT INTO item (item_id) VALUES ('" + wInput + L"')";

	rtrn = SQLExecDirect(handle, (SQLWCHAR*)insertQuery.c_str());

	if (SQL_SUCCEEDED(rtrn))
	{
		cout << "Insert successful!" << endl;
	}
	else
	{
		cerr << "Insert failed!" << endl;
	}
}
