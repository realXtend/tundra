#pragma once

#include <Windows.h>
#include <string>
#include <list>

/// Threaded stdin input console
class InputConsole
{
public:
    InputConsole();
    ~InputConsole();
    
    /// Read a line of console input, if available
    /// @param line Where to read
    /// @return true If had input, false if not
    bool ReadInput(std::string& line);
    
private:
    static DWORD WINAPI ThreadFunction(void* userData);
    void RunInputLoop();
    void GetCS();
    void ReleaseCS();
    
    HANDLE inputThread;
    CRITICAL_SECTION inputCS;
    std::list<std::string> lines;
    volatile bool shouldRun;
    volatile bool hasInput;
};