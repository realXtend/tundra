// For conditions of distribution and use, see copyright notice in license.txt

#include "Framework.h"
#include "CoreDefines.h"
#include "LoggingFunctions.h"
#include "FrameAPI.h"

#include "BrowserUiPlugin.h"
#include "CookieJar.h"

#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include <QScriptEngine>

Q_DECLARE_METATYPE(CookieJar*);

BrowserUiPlugin::BrowserUiPlugin(Framework *framework) :
    QObject(0),
    framework_(framework),
    startUrlChecked_(false)
{
    JavascriptModule *jsModule = framework_->GetModule<JavascriptModule>();
    if (jsModule)
        connect(jsModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
    else
        LogWarning("BrowserUiPlugin: Could not get JavascriptModule to connect to the engine created signal!");

    connect(framework_->Frame(), SIGNAL(Updated(float)), this, SLOT(CheckStartParamUrl()));
}

BrowserUiPlugin::~BrowserUiPlugin()
{
}

void BrowserUiPlugin::CheckStartParamUrl()
{
    if (startUrlChecked_)
        return;
    startUrlChecked_ = true;
    disconnect(framework_->Frame(), SIGNAL(Updated(float)), this, SLOT(CheckStartParamUrl()));

    // If a login url was provided but without the username query, lets open the page up in the browser.
    // This is a way for 3rd party apps to open Tundra with a custom start web page, without doing a actual login yet.
    QStringList cmdLineParams = framework_->CommandLineParameters("--login");
    if (cmdLineParams.size() > 0)
    {
        QUrl url(cmdLineParams.first(), QUrl::TolerantMode);
        if (url.isValid())
        {
            QString username = url.queryItemValue("username").trimmed();
            if (username.isEmpty())
            {
                QString urlScheme = url.scheme().toLower();
                if (urlScheme == "tundra")
                    url.setScheme("http");
                OpenUrl(url, true);
            }
        }
    }
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
