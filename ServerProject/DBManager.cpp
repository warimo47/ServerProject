#include "DBManager.h"

#include "CharactersManager.h"

DBManager::DBManager()
{
	SQLHENV henv = nullptr;
	SQLHDBC hdbc = nullptr;

	ZeroMemory(dbQueryBuffer, sizeof(dbQueryBuffer));

	// Allocate environment handle  
	SQLRETURN sqlReturn = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(henv, SQL_HANDLE_ENV, sqlReturn);
	}

	// Set the ODBC version environment attribute  
	sqlReturn = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(henv, SQL_HANDLE_ENV, sqlReturn);
	}

	// Allocate connection handle  
	sqlReturn = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(henv, SQL_HANDLE_ENV, sqlReturn);
	}

	// Set login timeout to 5 seconds  
	sqlReturn = SQLSetConnectAttr(hdbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, sqlReturn);
	}

	charactersManager = nullptr;
}

DBManager::~DBManager() {}

void DBManager::SetCharactersManagerPointer(CharactersManager* cm)
{
	charactersManager = cm;
}

void DBManager::HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode)
{
	SQLSMALLINT iRec = 0;
	SQLINTEGER iError;
	WCHAR wszMessage[1000];
	WCHAR wszState[SQL_SQLSTATE_SIZE + 1];

	if (RetCode == SQL_INVALID_HANDLE)
	{
		fwprintf(stderr, L"Invalid handle!\n");
		return;
	}
	while (SQLGetDiagRec(hType, hHandle, ++iRec, wszState, &iError, wszMessage, (SQLSMALLINT)(sizeof(wszMessage) / sizeof(WCHAR)), (SQLSMALLINT *)NULL) == SQL_SUCCESS)
	{
		// Hide data truncated..
		if (wcsncmp(wszState, L"01004", 5))
		{
			fwprintf(stderr, L"[%5.5s] %s (%d)\n", wszState, wszMessage, iError);
		}
		std::wcout << wszMessage;
	}
	std::wcout << "\n";
}

void DBManager::SaveDB(std::wstring userID_wstr)
{
	// Connect to data source  
	SQLRETURN sqlReturn = SQLConnect(hdbc, (SQLWCHAR*)L"NPS", 3, (SQLWCHAR*)L"warimo47", 8, (SQLWCHAR*)L"dkssud!123", 10);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, sqlReturn);
	}

	// Allocate statement handle  
	sqlReturn = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, sqlReturn);
	}

	wchar_t idStr[10];
	int level = 0; int positionX = 0; int positionY = 0; int expeiencePoint = 0; int healthPoint = 0;

	wsprintf(dbQueryBuffer, L"EXEC dbo.saveUserInformation %ws, %d, %d, %d, %d, %d",
		idStr, level, positionX, positionY, expeiencePoint, healthPoint);
	//wsprintf(g_DBQueryBuffer, L"EXEC dbo.AddClient asdkv");

	sqlReturn = SQLExecDirect(hstmt, dbQueryBuffer, SQL_NTS);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, sqlReturn);
	}

	sqlReturn = SQLDisconnect(hdbc);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, sqlReturn);
	}
}

bool DBManager::TryLogin(wchar_t* userID_wstr, int cKey)
{
	// Connect to data source  
	SQLRETURN sqlReturn = SQLConnect(hdbc, (SQLWCHAR*)L"NPS", 3, (SQLWCHAR*)L"warimo47", 8, (SQLWCHAR*)L"dkssud!123", 10);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, sqlReturn);
		return false;
	}

	// Allocate statement handle  
	sqlReturn = SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, sqlReturn);
		return false;
	}

	SQLINTEGER sqlIntLevel = 0;
	SQLINTEGER sqlIntPositionX = 0;
	SQLINTEGER sqlIntPositionY = 0;
	SQLINTEGER sqlIntExperiencePoint = 0;
	SQLINTEGER sqlIntHealthPoint = 0;
	SQLLEN sqlLenLevel = 0;
	SQLLEN sqlLenPositionX = 0;
	SQLLEN sqlLenPositionY = 0;
	SQLLEN sqlLenExperiencePoint = 0;
	SQLLEN sqlLenHealthPoint = 0;

	wsprintf(dbQueryBuffer, L"EXEC dbo.tryLogin %ws", userID_wstr);

	sqlReturn = SQLExecDirect(hstmt, dbQueryBuffer, SQL_NTS);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, sqlReturn);
		return false;
	}

	// Bind columns
	sqlReturn = SQLBindCol(hstmt, 1, SQL_INTEGER, &sqlIntLevel, 1, &sqlLenLevel);
	sqlReturn = SQLBindCol(hstmt, 2, SQL_INTEGER, &sqlIntPositionX, 1, &sqlLenPositionX);
	sqlReturn = SQLBindCol(hstmt, 3, SQL_INTEGER, &sqlIntPositionY, 1, &sqlLenPositionY);
	sqlReturn = SQLBindCol(hstmt, 4, SQL_INTEGER, &sqlIntExperiencePoint, 1, &sqlLenExperiencePoint);
	sqlReturn = SQLBindCol(hstmt, 5, SQL_INTEGER, &sqlIntHealthPoint, 1, &sqlLenHealthPoint);
	// HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, sqlReturn);

	// Fetch and print each row of data. On an error, display a message and exit.
	sqlReturn = SQLFetch(hstmt);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hstmt, SQL_HANDLE_STMT, sqlReturn);
		return false;
	}
	
	// Connect new user
	charactersManager->UserLogIn(userID_wstr,
		static_cast<int>(sqlIntLevel),
		static_cast<int>(sqlIntPositionX),
		static_cast<int>(sqlIntPositionY),
		static_cast<int>(sqlIntExperiencePoint),
		static_cast<int>(sqlIntHealthPoint),
		cKey);

	// Disconnect SQL
	sqlReturn = SQLDisconnect(hdbc);
	if (sqlReturn == SQL_ERROR || sqlReturn == SQL_SUCCESS_WITH_INFO)
	{
		HandleDiagnosticRecord(hdbc, SQL_HANDLE_DBC, sqlReturn);
		return false;
	}

	return true;
}