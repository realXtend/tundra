// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_MumbleLibrary_h
#define incl_MumbleVoipModule_MumbleLibrary_h

#include <QObject>
#include <QString>

namespace MumbleLib
{
    class MumbleMainLoopThread;

    //! @todo Singleton instance
    class MumbleLibrary : public QObject
    {
        Q_OBJECT
        MumbleLibrary();
    public:
        //! Start Mumble library in separate thread if it isn't alraedy running
        static void Start();

        //! Stop Mumble library thread if it's running
        static void Stop();

        //! @return true if mumble library thread is running, otherwise false
        static bool IsRunning();

        //! Return reason for current state
        static QString Reason();

        static QThread* MainLoopThread();

    //signals:
    //    static void Started();
    //    static void Stoped(); 
    //    static void InternalError();

    private:
        static void StartMumbleThread();
        static void StopMumbleThread();
        static QString reason_;
        static MumbleMainLoopThread* mumble_main_loop_;
    };

} // MumbleLib

#endif // incl_MumbleVoipModule_MumbleLibrary_h