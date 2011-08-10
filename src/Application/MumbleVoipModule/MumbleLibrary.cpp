// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CoreDefines.h"

#include "MumbleLibrary.h"
#include "MumbleMainLoopThread.h"

#include "LibMumbleClient.h"

#include "MemoryLeakCheck.h"

namespace MumbleLib
{
    QString MumbleLibrary::reason_ = "";
    MumbleMainLoopThread* MumbleLibrary::mumble_main_loop_ = 0;

    void MumbleLibrary::Start()
    {
        StartMumbleThread();
    }

    void MumbleLibrary::Stop()
    {
        StopMumbleThread();
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
        }

        if (mumble_main_loop_->isRunning())
            return;

        /// @todo Proper shutdown, or what does this mean?
        //connect(lib_mumble_thread_, SIGNAL(finished()), SLOT(MumbleThreadFinished()) );
        //connect(lib_mumble_thread_, SIGNAL(terminated()), SLOT(MumbleThreadFinished()) );
                
        mumble_main_loop_->start();
        mumble_main_loop_->setPriority(QThread::LowPriority);
    }

    void MumbleLibrary::StopMumbleThread()
    {
        if (!mumble_main_loop_)
            return;

        ::MumbleClient::MumbleClientLib* mumble_lib = ::MumbleClient::MumbleClientLib::instance();
        if (!mumble_lib)
            return;

        mumble_lib->Shutdown();
        mumble_main_loop_->wait();
        SAFE_DELETE(mumble_main_loop_);
    }

    QThread* MumbleLibrary::MainLoopThread()
    {
        return mumble_main_loop_;
    }

}
