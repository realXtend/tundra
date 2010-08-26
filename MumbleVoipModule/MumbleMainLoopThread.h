// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_MumbleMainLoopThread_h
#define incl_MumbleVoipModule_MumbleMainLoopThread_h

#include <QObject>
#include <QThread>
#include "CoreTypes.h"
#include "Framework.h"

namespace MumbleLib
{
    /**
     * Thread for running the lib mumble mainloop
     * 
     */
    class MumbleMainLoopThread : public QThread 
    {
    public:
        virtual void run();

        //! \return information about current state
        virtual QString Reason() const;
    private:
        QString reason_;
    };
} // end of namespace: MumbleLib

#endif // incl_MumbleVoipModule_MumbleMainLoopThread_h
