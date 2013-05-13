// For conditions of distribution and use, see copyright notice in LICENSE

#include "TundraCoreApi.h"

int TUNDRACORE_API run(int argc, char **argv);

#if defined(_WIN64) && defined(_DEBUG)
#include <kNet/64BitAllocDebugger.h>
BottomMemoryAllocator bma; // Use kNet's BottomMemoryAllocator on to catch potential 64-bit memory allocation bugs early on.
#endif

int main(int argc, char **argv)
{
    return run(argc, argv);
}
