#include "StableHeaders.h"

#include "LoggingFunctions.h"
#include "Framework.h"
#include "ConsoleAPI.h"

#include "Win.h"

#ifdef ANDROID
#include <android/log.h>
#endif

void PrintLogMessage(u32 logChannel, const char *str)
{
    if (!IsLogChannelEnabled(logChannel))
        return;

    Framework *instance = Framework::Instance();
    ConsoleAPI *console = (instance ? instance->Console() : 0);

    // On Windows, highlight errors and warnings.
#ifdef WIN32
    if ((logChannel & LogChannelError) != 0) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
    else if ((logChannel & LogChannelWarning) != 0) SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
#endif
    // The console and stdout prints are equivalent.
    if (console)
        console->Print(str);
    else // The Console API is already dead for some reason, print directly to stdout to guarantee we don't lose any logging messags.
    {
        #ifndef ANDROID
            printf("%s", str);
        #else
            __android_log_print(ANDROID_LOG_INFO, "Tundra", "%s", str);
        #endif
    }

    // Restore the text color to normal.
#ifdef WIN32
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
#endif
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

