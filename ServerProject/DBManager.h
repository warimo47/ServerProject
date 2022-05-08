#pragma once

#define WIN32_LEAN_AND_MEAN

#include <iostream>

#include <stdio.h>
#include <windows.h>
#include <sqlext.h>

class CharactersManager;

class DBManager
{
private:
	CharactersManager* charactersManager;

	SQLHDBC hdbc;
	SQLHSTMT hstmt;

	wchar_t dbQueryBuffer[256];

protected:

public:

private:

protected:

public:
	DBManager();
	~DBManager();

	void SetCharactersManagerPointer(CharactersManager* cm);

	void HandleDiagnosticRecord(SQLHANDLE hHandle, SQLSMALLINT hType, RETCODE RetCode);

	void SaveDB(std::wstring userID_wstr);
	bool TryLogin(wchar_t* userID_wstr, int cKey);
};

