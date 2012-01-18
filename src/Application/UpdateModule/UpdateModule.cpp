// For conditions of distribution and use, see copyright notice in LICENSE

#include "UpdateModule.h"

#include "Framework.h"
#include "CoreDefines.h"
#include "Application.h"

#include <QDir>
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

void UpdateModule::Initialize()
{
    if (!GetFramework()->IsHeadless())
        RunUpdater("/silent");
}

void UpdateModule::RunUpdater(QString parameter)
{
#ifdef Q_WS_WIN
    QDir installDir(QDir::fromNativeSeparators(Application::InstallationDirectory()));
    QFile executable(installDir.absoluteFilePath(updateExecutable));
    QFile config(installDir.absoluteFilePath(updateConfig));
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
