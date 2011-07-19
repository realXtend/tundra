// For conditions of distribution and use, see copyright notice in license.txt

#include "UpdateModule.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include <QFile>
#include <QProcess>
#include <QAction>
#include <QMenu>

UpdateModule::UpdateModule() :
    IModule("UpdateModule"),
    updateExecutable("./TundraUpdater.exe"),
    updateConfig("./TundraUpdater.ini")
{
}

UpdateModule::~UpdateModule()
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

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new UpdateModule();
        fw->RegisterModule(module);
    }
}
