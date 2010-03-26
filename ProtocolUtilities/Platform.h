// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_Protocol_Platform_h
#define incl_Protocol_Platform_h

#ifdef WIN32
#include <Windows.h>

// The following macros set and clear, respectively, given bits
// of the C runtime library debug flag, as specified by a bitmask.
#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

// This routine place comments at the head of a section of debug output
void OutputHeading(const char *explanation)
{
    _RPT1( _CRT_WARN, "\n\n%s:\n**************************************************************************\n", explanation );
}

void HeapDebug(_CrtMemState *s1, _CrtMemState *s2)
{
    _CrtMemState s3;
    // Now all the request memory is unnecessary.
#ifndef _DEBUG
    printf("Skipping memory heap checking in non-debug mode.\n");
    return;
#endif
    if (_CrtMemDifference(&s3, s1, s2))
    {
        OutputHeading("There are memory leaks. Changes between two memory checkpoints");
        _CrtMemDumpStatistics(&s3);
    }
}

#endif // WIN32

#endif // incl_Protocol_Platform_h
