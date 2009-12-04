#ifndef incl_Communication_TelepathyIM_GMainLoopThread_h
#define incl_Communication_TelepathyIM_GMainLoopThread_h

#include <glib-object.h>
#include <gst/gst.h>
#include <glib.h>

#include <QThread>
//#include <TelepathyQt4/Types>
#include <TelepathyQt4/Connection>
#include <TelepathyQt4/StreamedMediaChannel>
#include <TelepathyQt4/PendingReady>

              
namespace TelepathyIM
{
	/**
	 *       
	 */
	class GMainLoopThread : public QThread
    {
    public:
        GMainLoopThread();
        virtual void run();
        virtual void Stop();

    private:
           GMainLoop* g_main_loop_;
    };

} // end of namespace

#endif // incl_Communication_TelepathyIM_GMainLoopThread_h
