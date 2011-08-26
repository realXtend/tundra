#include "StableHeaders.h"

#include "LoggingFunctions.h"
#include "Framework.h"
#include "ConsoleAPI.h"

void PrintLogMessage(u32 logChannel, const char *str)
{
    if (!IsLogChannelEnabled(logChannel))
        return;

    Framework *instance = Framework::Instance();
    ConsoleAPI *console = (instance ? instance->Console() : 0);

    // Use console if available, it will print to console widget (if available) and to stdout.
    // Otherwise use normal stdout only (this happens before ConsoleAPI is initialized).
    if (console)
        console->Print(str);
    else
        printf("%s", str);
}

bool IsLogChannelEnabled(u32 logChannel)
{
    Framework *instance = Framework::Instance();
    ConsoleAPI *console = (instance ? instance->Console() : 0);

    // If console is null, we've already killed Framework and ConsoleAPI or ConsoleAPI has not yet been initialized! 
    // Print out everything so that we can't accidentally lose any important messages.
    if (console)
        return console->IsLogChannelEnabled(logChannel);
    else
        return true; 
}

