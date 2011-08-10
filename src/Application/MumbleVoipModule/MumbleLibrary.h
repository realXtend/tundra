// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "MumbleFwd.h"

#include <QObject>
#include <QString>

namespace MumbleLib
{
    //! @todo Singleton instance
    class MumbleLibrary : public QObject
    {

    Q_OBJECT
    
    public:
        MumbleLibrary();

        //! Start Mumble library in separate thread if it isn't alraedy running
        static void Start();

        //! Stop Mumble library thread if it's running
        static void Stop();

        //! @return true if mumble library thread is running, otherwise false
        static bool IsRunning();

        //! Return reason for current state
        static QString Reason();

        //! Return the main thread
        static QThread* MainLoopThread();

    private:
        static void StartMumbleThread();
        static void StopMumbleThread();
        static QString reason_;
        static MumbleMainLoopThread* mumble_main_loop_;
    };
}
