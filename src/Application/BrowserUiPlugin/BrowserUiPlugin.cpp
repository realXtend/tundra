// For conditions of distribution and use, see copyright notice in license.txt

#include "Framework.h"
#include "CoreDefines.h"
#include "LoggingFunctions.h"

#include "BrowserUiPlugin.h"
#include "CookieJar.h"

#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include <QScriptEngine>

Q_DECLARE_METATYPE(CookieJar*);

BrowserUiPlugin::BrowserUiPlugin(Framework *framework) :
    QObject(0),
    framework_(framework)
{
    JavascriptModule *jsModule = framework_->GetModule<JavascriptModule>();
    if (jsModule)
        connect(jsModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
    else
        LogWarning("BrowserUiPlugin: Could not get JavascriptModule to connect to the engine created signal!");
}

BrowserUiPlugin::~BrowserUiPlugin()
{
}

void BrowserUiPlugin::OnScriptEngineCreated(QScriptEngine *engine)
{
    qScriptRegisterQObjectMetaType<CookieJar*>(engine);
}

void BrowserUiPlugin::AddAction(QAction *action, const QString &group)
{
    emit ActionAddRequest(action, group);
}

void BrowserUiPlugin::OpenUrl(const QString &url, bool activateNewTab)
{
    OpenUrl(QUrl::fromUserInput(url), activateNewTab);
}

void BrowserUiPlugin::OpenUrl(const QUrl &url, bool activateNewTab)
{
    emit OpenUrlRequest(url, activateNewTab);
}

CookieJar *BrowserUiPlugin::CreateCookieJar(const QString &cookieDiskFile)
{
    return new CookieJar(this, cookieDiskFile);
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        BrowserUiPlugin *plugin = new BrowserUiPlugin(fw);
        fw->RegisterDynamicObject("browserplugin", plugin);
    }
}
