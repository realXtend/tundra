#include "GMainLoopThread.h"
              
namespace TelepathyIM
{
    GMainLoopThread::GMainLoopThread()
    {
        g_main_loop_ = g_main_loop_new (NULL, FALSE);
    }
    
    void GMainLoopThread::run()
    {
        g_main_loop_run (g_main_loop_);
    }

    void GMainLoopThread::Stop()
    {
        g_main_loop_quit(g_main_loop_);
    }

} // end of namespace: TelepathyIM
