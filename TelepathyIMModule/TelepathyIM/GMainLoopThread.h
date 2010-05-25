// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_GMainLoopThread_h
#define incl_Communication_TelepathyIM_GMainLoopThread_h

//#include <Foundation.h>
//#include <glib-object.h>
//#include <gst/gst.h>
//#include <glib.h>
#include <glib/gmain.h>

#include <QThread>
//#include <TelepathyQt4/Types>
//#include <TelepathyQt4/Connection>
//#include <TelepathyQt4/StreamedMediaChannel>
//#include <TelepathyQt4/PendingReady>

#include "ModuleLoggingFunctions.h"


namespace TelepathyIM
{
    /**
     *       
     */
    class GMainLoopThread : public QThread
    {
        MODULE_LOGGING_FUNCTIONS
        static const std::string NameStatic() { return "TelepathyIM"; } // for logging functionality
    public:
        GMainLoopThread();
        virtual void run();
        virtual void StopLoop();

    //private:
           GMainLoop* g_main_loop_;
    };

} // end of namespace

#endif // incl_Communication_TelepathyIM_GMainLoopThread_h
