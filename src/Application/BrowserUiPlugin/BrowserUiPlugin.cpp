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
#include <QNetworkDiskCache>

#include "QScriptEngineHelpers.h"

Q_DECLARE_METATYPE(CookieJar*);

BrowserUiPlugin::BrowserUiPlugin() :
    IModule("BrowserUiPlugin"),
    reserverCookieFile_("cookies.data"),
    mainCookieJar_(0),
    mainDiskCache_(0)
{
}

BrowserUiPlugin::~BrowserUiPlugin()
{
    if (mainCookieJar_)
    {
        mainCookieJar_->setParent(0);
        SAFE_DELETE(mainCookieJar_);
    }
    if (mainDiskCache_)
    {
        mainDiskCache_->setParent(0);
        SAFE_DELETE(mainDiskCache_);
    }
}

void BrowserUiPlugin::Load()
{
    // For headless instances we don't init or
    // expose our mainly UI related functionality.
    if (framework_->IsHeadless())
        return;

    // Determine the data dir
    QString subdir = "browsercache";
    QDir appData(Application::UserDataDirectory());
    if (!appData.exists(subdir))
        appData.mkdir(subdir);
    appData.cd(subdir);
    dataDir_ = appData.absolutePath();

    // Create main cookie file.
    reservedCookiePath_ = appData.absoluteFilePath(reserverCookieFile_);
    mainCookieJar_ = new CookieJar(0, reservedCookiePath_);

    // Create main disk cache.
    mainDiskCache_ = new QNetworkDiskCache(0);
    mainDiskCache_->setCacheDirectory(dataDir_);

    framework_->RegisterDynamicObject("browserplugin", this);
}

void BrowserUiPlugin::Initialize()
{
    JavascriptModule *jsModule = framework_->GetModule<JavascriptModule>();
    if (jsModule)
        connect(jsModule, SIGNAL(ScriptEngineCreated(QScriptEngine*)), SLOT(OnScriptEngineCreated(QScriptEngine*)));
    else
        LogWarning("BrowserUiPlugin: Could not get JavascriptModule to connect to the engine created signal!");
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

void BrowserUiPlugin::SetBrowserVisibility(bool visible)
{
    emit SetBrowserVisibilityRequest(visible);
}

CookieJar *BrowserUiPlugin::MainCookieJar()
{
    // Help out to properly use the cookies with reseting parent
    if (mainCookieJar_)
        mainCookieJar_->setParent(0);
    return mainCookieJar_.data();
}

QNetworkDiskCache *BrowserUiPlugin::MainDiskCache()
{
    // Help out to properly use the cache with reseting parent
    if (mainDiskCache_)
        mainDiskCache_->setParent(0);
    return mainDiskCache_.data();
}

CookieJar *BrowserUiPlugin::CreateCookieJar(const QString &cookieDiskFile)
{
    // If has the same filename as reserved path, look into it more
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
        IModule *module = new BrowserUiPlugin();
        fw->RegisterModule(module);
    }
}
