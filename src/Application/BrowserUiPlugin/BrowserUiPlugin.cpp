// For conditions of distribution and use, see copyright notice in license.txt

#include "Framework.h"
#include "CoreDefines.h"
#include "Application.h"
#include "LoggingFunctions.h"

#include "BrowserUiPlugin.h"
#include "CookieJar.h"

#include "JavascriptModule.h"
#include "ScriptMetaTypeDefines.h"
#include <QScriptEngine>

#include "QScriptEngineHelpers.h"

Q_DECLARE_METATYPE(CookieJar*);

BrowserUiPlugin::BrowserUiPlugin(Framework *framework) :
    framework_(framework),
    reserverCookieFile_("cookies.data"),
    mainCookieJar_(0)
{
    // Determine the reserver cookie file to appdata.
    QString subdir = "browsercache";
    QDir appData(Application::UserDataDirectory());
    if (!appData.exists(subdir))
        appData.mkdir(subdir);
    appData.cd(subdir);
    reservedCookiePath_ = appData.absoluteFilePath(reserverCookieFile_);
    
    // Create main cookie.
    mainCookieJar_ = CreateCookieJar(reservedCookiePath_);

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

CookieJar *BrowserUiPlugin::MainCookieJar()
{
    return mainCookieJar_;
}

CookieJar *BrowserUiPlugin::CreateCookieJar(const QString &cookieDiskFile)
{
    // If has the same filename as preserved path, look into it more
    if (cookieDiskFile.endsWith(reserverCookieFile_, Qt::CaseInsensitive))
    {
        QFileInfo inInfo(cookieDiskFile);
        QFileInfo reserverInfo(reservedCookiePath_);
        if (inInfo.absoluteFilePath().toLower() == reserverInfo.absoluteFilePath().toLower())
        {
            LogWarning("BrowserUiPlugin::CreateCookieJar: Tried to re-create main cookie jar, returning existing jar. Use BrowserUiPlugin::MainCookieJar() instead!");
            return MainCookieJar();
        }
    }
    return new CookieJar(this, cookieDiskFile);
}

void BrowserUiPlugin::ShowProgressScreen(QString message)
{
    emit ShowProgressScreenRequest(message);
}

void BrowserUiPlugin::HideProgressScreen()
{
    emit HideProgressScreenRequest();
}

void BrowserUiPlugin::UpdateProgressScreen(QString message, int progress)
{
    emit UpdateProgressScreenRequest(message, progress);
}

void BrowserUiPlugin::UpdateProgressScreenImage(QString absoluteFilePath)
{
    if (absoluteFilePath.isEmpty())
    {
        QImage img(300, 100, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);
        emit UpdateProgressImageRequest(img);
    }
    else if (QFile::exists(absoluteFilePath))
    {
        QImage img(absoluteFilePath);
        if (!img.isNull())
        {
            if (img.width() > 300)
                img = img.scaledToWidth(300);
            if (img.height() > 100)
                img = img.scaledToHeight(100);
            emit UpdateProgressImageRequest(img);
        }
        else
            LogWarning("BrowserUiPlugin::SetProgressScreenImage: Failed to open image '" + absoluteFilePath + "'");
    }
    else
        LogWarning("BrowserUiPlugin::SetProgressScreenImage: Given file does not exist '" + absoluteFilePath + "'");
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
