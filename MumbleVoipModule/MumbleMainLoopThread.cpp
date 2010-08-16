// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MumbleMainLoopThread.h"
#include "MumbleVoipModule.h"

#define BUILDING_DLL // for dll import/export declarations
#define CreateEvent  CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL // for dll import/export declarations

#include "MemoryLeakCheck.h"

namespace MumbleVoip
{
    void MumbleMainLoopThread::run()
    {
        MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
            return;
        }
        MumbleVoipModule::LogDebug("Mumble library mainloop started");
        try
        {
            mumble_lib->Run();
        }
        catch(std::exception &e)
        {
            QString message = QString("Mumble library mainloop stopped by exception: %1").arg(e.what());
            MumbleVoipModule::LogError(message.toStdString());
        }
        catch(...)
        {
            QString message = QString("Mumble library mainloop stopped by unknown exception.");
            MumbleVoipModule::LogError(message.toStdString());
            reason_ = message;
        }
        MumbleVoipModule::LogDebug("Mumble library mainloop stopped");
    }

    QString MumbleMainLoopThread::Reason() const
    {
        return reason_;
    }

} // MumbleVoip
