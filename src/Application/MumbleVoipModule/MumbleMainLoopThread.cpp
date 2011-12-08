// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MumbleMainLoopThread.h"
#include "MumbleVoipModule.h"
#include "LibMumbleClient.h"
#include "LoggingFunctions.h"

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
        catch (std::exception &e)
        {
            reason_ = e.what();
            LogError("Mumble library mainloop stopped with exception: " + std::string(e.what()));
        }
        catch (...)
        {
            reason_ = "Mumble library mainloop stopped with unknown exception.";
            LogError(reason_);
        }

        LogDebug("Mumble library main loop stopped");
    }

    QString MumbleMainLoopThread::Reason() const
    {
        return reason_;
    }

}
