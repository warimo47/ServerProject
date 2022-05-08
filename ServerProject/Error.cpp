#include <iostream>

#include "Error.h"

void ErrorDisplay(const wchar_t * location)
{
	int errorNumber = WSAGetLastError();
	WCHAR* errorMessageBuffer = nullptr;

	if (errorNumber == WSA_IO_PENDING)
	{
		// std::wcout << location << L"WSA ID PENDING\n";
		return;
	}

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, 
		errorNumber, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&errorMessageBuffer, 0, NULL);

	std::wcout << location << errorMessageBuffer << std::endl;
	LocalFree(errorMessageBuffer);

	while (true);
}