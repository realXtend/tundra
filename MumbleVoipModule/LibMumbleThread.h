// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_LibMumbleMainLoopThread_h
#define incl_MumbleVoipModule_LibMumbleMainLoopThread_h

#include <QObject>
#include <QThread>
#include "CoreTypes.h"
#include "Framework.h"

namespace MumbleVoip
{
    /**
     * Thread for running the lib mumble mainloop
     *
     */
    class LibMumbleThread : public QThread 
    {
    public:
        virtual void run();

        //! \return information about current state
        virtual QString Reason() const;
    private:
        QString reason_;
    };
} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_LibMumbleMainLoopThread_h
