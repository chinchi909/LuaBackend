#ifndef CONSOLELIB
#define CONSOLELIB

#include <iostream>
#include <string>

#include <sol/sol.hpp>
#include <any>

class ConsoleLib
{
	public:
    static void MessageOutput(string Text, int MessageType)
    {
        HANDLE _hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

        switch (MessageType)
        {
        case 0:
            SetConsoleTextAttribute(_hConsole, 11);
            cout << "MESSAGE: ";
            break;
        case 1:
            SetConsoleTextAttribute(_hConsole, 10);
            cout << "SUCCESS: ";
            break;
        case 2:
            SetConsoleTextAttribute(_hConsole, 14);
            cout << "WARNING: ";
            break;
        case 3:
            SetConsoleTextAttribute(_hConsole, 12);
            cout << "ERROR: ";
            break;
        }

        SetConsoleTextAttribute(_hConsole, 7);
        cout << Text;
    }
};

#endif
