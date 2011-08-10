// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "LoggingFunctions.h"

#include "MumbleMainLoopThread.h"
#include "MumbleVoipModule.h"

#include "LibMumbleClient.h"

#include "MemoryLeakCheck.h"

namespace MumbleLib
{
    void MumbleMainLoopThread::run()
    {
        ::MumbleClient::MumbleClientLib* mumble_lib = ::MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
            return;
        
        LogDebug("Mumble library main loop started");
        
        try
        {
            mumble_lib->SetLogLevel(::MumbleClient::logging::LOG_FATAL);
            mumble_lib->Run();
        }
        catch(std::exception &e)
        {
            LogError(QString("Mumble library mainloop stopped by exception: %1").arg(e.what()));
        }
        catch(...)
        {
            reason_ = "Mumble library mainloop stopped by unknown exception.";
            LogError(reason_);
        }
        LogDebug("Mumble library main loop stopped");
    }

    QString MumbleMainLoopThread::Reason() const
    {
        return reason_;
    }

}
