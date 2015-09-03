// UnitTests.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h> // WinApi header
#include "Common/Half.hpp"

void printResult(HANDLE hConsole, WORD defaultAttribute, bool result)
{
	if(result)
	{
		SetConsoleTextAttribute(hConsole, 0x2); // 0x2 = green
		std::cout << "passed";
	}
	else
	{
		SetConsoleTextAttribute(hConsole, 0x4); // 0x4 = red
		std::cout << "failed";
	}

	SetConsoleTextAttribute(hConsole, defaultAttribute);
}

int _tmain(int argc, _TCHAR* argv[])
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	// Get default console attribute
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	WORD currentConsoleAttr = 0xF; // 0xF = white
	if(GetConsoleScreenBufferInfo(hConsole, &csbi))
		currentConsoleAttr = csbi.wAttributes;

	// Starting tests
	std::cout << "Running unit tests..." << std::endl << std::endl;

	// Half to float / Float to half conversion tests
	std::cout << "* Half <-> Float conversion test ";
	printResult(hConsole, currentConsoleAttr, sw::half::TestConversionEquivalence());
	std::cout << std::endl;

	// Tests done
	std::cout << std::endl << "Tests completed. Press enter to exit" << std::endl;
	std::cin.get();
	return 0;
}

