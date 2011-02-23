// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UpdateModule.h"

#include "NaaliUi.h"
#include "NaaliMainWindow.h"

#include <QFile>
#include <QProcess>
#include <QAction>
#include <QMenu>
#include <QDebug>

namespace Update
{
    static std::string module_name = "UpdateModule";
    const std::string &UpdateModule::NameStatic() { return module_name; }

    UpdateModule::UpdateModule() :
        QObject(),
        IModule(module_name),
        updateExecutable("./TundraUpdater.exe"),
        updateConfig("./TundraUpdater.ini")
    {
    }

    UpdateModule::~UpdateModule()
    {
    }

    void UpdateModule::Load()
    {
    }

    void UpdateModule::PostInitialize()
    {
        if (!GetFramework()->IsHeadless())
            RunUpdater("/silent");
    }
    
    void UpdateModule::RunUpdater(QString parameter)
    {
#ifdef Q_WS_WIN
        QFile executable(updateExecutable);
        QFile config(updateConfig);
        if (executable.exists() && config.exists())
            QProcess::startDetached(executable.fileName(), QStringList() << parameter);
#endif
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace Update;
POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(UpdateModule)
POCO_END_MANIFEST
