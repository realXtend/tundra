#include "StableHeaders.h"
#include "MumbleLibrary.h"
#include "MumbleMainLoopThread.h"
#define BUILDING_DLL // for dll import/export declarations
#define CreateEvent  CreateEventW // for \boost\asio\detail\win_event.hpp and \boost\asio\detail\win_iocp_handle_service.hpp
#include <mumbleclient/client_lib.h>
#undef BUILDING_DLL // for dll import/export declarations

namespace MumbleVoip
{
    QString MumbleLibrary::reason_ = "";
    MumbleMainLoopThread* MumbleLibrary::mumble_main_loop_ = 0;

    void MumbleLibrary::Start()
    {
        StartMumbleThread();
        //emit Started();
    }

    void MumbleLibrary::Stop()
    {
//        emit Stoped();
    }

    bool MumbleLibrary::IsRunning()
    {
        if (!mumble_main_loop_)
            return false;
        return mumble_main_loop_->isRunning();
    }

    QString MumbleLibrary::Reason()
    {
        return reason_;
    }

    void MumbleLibrary::StartMumbleThread()
    {
        if (!mumble_main_loop_)
        {
            mumble_main_loop_ = new MumbleMainLoopThread();
//            lib_mumble_thread_->moveToThread(QThread::currentThread()); // for cross thread signaling
        }

        if (mumble_main_loop_->isRunning())
            return;

        //! @todo
        //connect(lib_mumble_thread_, SIGNAL(finished()), SLOT(MumbleThreadFinished()) );
        //connect(lib_mumble_thread_, SIGNAL(terminated()), SLOT(MumbleThreadFinished()) );
                
        mumble_main_loop_->start();
        mumble_main_loop_->setPriority(QThread::LowPriority);
    }

    void MumbleLibrary::StopMumbleThread()
    {
        if (!mumble_main_loop_)
            return;

        MumbleClient::MumbleClientLib* mumble_lib = MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
        {
        //    MumbleVoipModule::LogError("Cannot stop Mumble library: No library instance available.");
            return;
        }

        mumble_lib->Shutdown();

        mumble_main_loop_->wait();
        SAFE_DELETE(mumble_main_loop_);
    }

} // MumbleVoip
