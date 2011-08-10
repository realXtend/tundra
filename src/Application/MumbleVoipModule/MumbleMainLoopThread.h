// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"
#include <QThread>

namespace MumbleLib
{
    /// Thread for running the lib mumble mainloop
    class MumbleMainLoopThread : public QThread 
    {
    public:
        virtual void run();
        //! \return information about current state
        virtual QString Reason() const;
    private:
        QString reason_;
    };
}
