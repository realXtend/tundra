#include "StableHeaders.h"

#include "LoggingFunctions.h"
#include "Framework.h"
#include "ConsoleAPI.h"

void PrintLogMessage(const char *str)
{
    Framework *instance = Framework::Instance();
    ConsoleAPI *console = (instance ? instance->Console() : 0);

    // The console and stdout prints are equivalent.
    printf("%s", str);
    if (console)
        console->Print(str);
}
