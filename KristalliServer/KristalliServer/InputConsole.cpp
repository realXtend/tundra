#include "InputConsole.h"

#include <iostream>

InputConsole::InputConsole() :
    shouldRun(true),
    hasInput(false),
    inputThread(0)
{
    InitializeCriticalSection(&inputCS);
    inputThread = CreateThread(0, 0, &ThreadFunction, this, 0, 0);
}

InputConsole::~InputConsole()
{
    shouldRun = false;
    
    WaitForSingleObject(inputThread, INFINITE);
    CloseHandle(inputThread);
    inputThread = 0;
    
    DeleteCriticalSection(&inputCS);
}

bool InputConsole::ReadInput(std::string& line)
{
    if (hasInput)
    {
        GetCS();
        line = lines.front();
        lines.pop_front();
        if (lines.empty())
            hasInput = false;
        ReleaseCS();
        
        return true;
    }
    
    return false;
}

DWORD WINAPI InputConsole::ThreadFunction(void* userData)
{
    ((InputConsole*)userData)->RunInputLoop();
    return 0;
}

void InputConsole::RunInputLoop()
{
    HANDLE inputHandle = GetStdHandle(STD_INPUT_HANDLE);
    while (shouldRun)
    {
        if (WaitForSingleObject(inputHandle, 0) == WAIT_OBJECT_0)
        {
            char buffer[256];
            std::cin.getline(buffer, 256);
            std::string line(buffer);
            if (line.length())
            {
                GetCS();
                lines.push_back(line);
                hasInput = true;
                ReleaseCS();
            }
        }
        Sleep(1);
    }
}

void InputConsole::GetCS()
{
    EnterCriticalSection(&inputCS);
}

void InputConsole::ReleaseCS()
{
    LeaveCriticalSection(&inputCS);
}

