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

    // The console and stdout prints are equivalent.
    if (console)
        console->Print(str);
    else // The Console API is already dead for some reason, print directly to stdout to guarantee we don't lose any logging messags.
        printf("%s", str);
}

bool IsLogChannelEnabled(u32 logChannel)
{
    Framework *instance = Framework::Instance();
    ConsoleAPI *console = (instance ? instance->Console() : 0);

    if (console)
        return console->IsLogChannelEnabled(logChannel);
    else
        return true; // We've already killed Framework and Console! Print out everything so that we can't accidentally lose any important messages.
}

