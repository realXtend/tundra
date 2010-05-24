// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "GMainLoopThread.h"

#include "MemoryLeakCheck.h"

namespace TelepathyIM
{
    GMainLoopThread::GMainLoopThread()
    {
        g_main_loop_ = g_main_loop_new (NULL, FALSE);
    }
    
    void GMainLoopThread::run()
    {
        LogDebug("g_main_loop started.");
        try
        {
            g_main_loop_run (g_main_loop_);
        }
        catch(...)
        {
            LogError("EXCEPTION inside gmainloop!");
        }
        LogDebug("g_main_loop ended.");
    }

    void GMainLoopThread::StopLoop()
    {
        g_main_loop_quit(g_main_loop_);
    }

} // end of namespace: TelepathyIM
