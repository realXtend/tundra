// For conditions of distribution and use, see copyright notice in license.txt

#include "FrameworkFwd.h"
#include "BrowserUiPluginApi.h"
#include "IModule.h"

#include <QObject>
#include <QString>
#include <QAction>
#include <QUrl>
#include <QImage>
#include <QPointer>

class CookieJar;
class QScriptEngine;
class QNetworkDiskCache;

class BROWSERUI_MODULE_API BrowserUiPlugin : public IModule
{

Q_OBJECT

public:
    /// Constructor.
    BrowserUiPlugin();

    /// Deconstructor.
    ~BrowserUiPlugin();

    /// IModule override.
    virtual void Load();

    /// IModule override.
    virtual void Initialize();

public slots:
    /// Adds a action to the browser ui.
    /// @param action Action to be added to the browser ui.
    /// @param group Additional group name, optional.
    void AddAction(QAction *action, const QString &group = QString());

    /// Opens url in the browser ui.
    /// @param url QString url.
    /// @param activateNewTab If the new tab where the url is opened should be activated or not. Optional, defaults to true.
    void OpenUrl(const QString &url, bool activateNewTab = true);

    /// Opens url in the browser ui.
    /// @param url QUrl url.
    /// @param activateNewTab If the new tab where the url is opened should be activated or not. Optional, defaults to true.
    void OpenUrl(const QUrl &url, bool activateNewTab = true);

    /// Sets the main browser ui visibility
    /// @param visible Boolean to set visibility.
    void SetBrowserVisibility(bool visible);

    /// Returns the main cookie jar that can be shared across Tundra functionality.
    /// Can be used with any QNetworkAccessManager with setCookieJar() function.
    /// @note Do not delete the returned object.
    /// @return CookieJar* Main cookie jar.
    CookieJar *MainCookieJar();

    /// Returns the main disk cache that can be shared across Tundra functionality.
    /// @note Do not delete the returned object. Be careful when using with QNetworkAccessManager as it takes ownership
    /// when setCache is called. When you delete your access manager you need to do the following accessManager->cache()->setParent(0).
    /// @return QNetworkDiskCache* Main disk cache.
    QNetworkDiskCache *MainDiskCache();

    /// Creates a new cookie jar that implements disk writing and reading. Can be used with any QNetworkAccessManager with setCookieJar() function.
    /// @note AssetCache will be the CookieJars parent and it will destroyed by it, don't take ownership of the returned CookieJar.
    /// Be careful when using with QNetworkAccessManager as it takes ownership when setCookieJar is called. 
    /// When you delete your access manager you need to do the following accessManager->cookieJar()->setParent(0).
    /// @param QString File path to the file the jar will read/write cookies to/from.
    /// @return CookieJar* Created cookie jar.
    CookieJar *CreateCookieJar(const QString &cookieDiskFile);

    /// Show progress screen with optional message.
    /// Call this upon login from a script in the scene to show a progress screen.
    void ShowProgressScreen(QString message = "");

    /// Hide progress screen. Call this when you feel like hiding the progress screen
    /// and showing the actual scene visuals. For example when avatar or main camera has been created and initialized.
    void HideProgressScreen();

    /// Update progress screen with message and optional progress 0-100.
    /// Default value for progress keeps infinite animation without precise value.
    /// Call this to update the message and progress in the loading screen.
    /// @note Passing >=100 to this function will not auto hide progress screen, you have to call HideProgressScreen() for that.
    void UpdateProgressScreen(QString message, int progress = -1);

    /// Set the progress screen image. Absolute filepath to the image file, 
    /// format support from Qt via QImage. Giving empty string will hide the image.
    /// @note Maximum image size is 300x100 px. Bigger images will be scaled down with trying to preserve the aspect ratio.
    void UpdateProgressScreenImage(QString absoluteFilePath);

private slots:
    void OnScriptEngineCreated(QScriptEngine *engine);

signals:
    /// Request to add a action. Connect to this if you are implementing a browser ui.
    /// @param action Action that wants to be added to browser ui.
    /// @param group Additional group name, can be empty a string.
    void ActionAddRequest(QAction *action, QString group);

    /// Request to open a url. Connect to this if you are implementing a browser ui.
    /// @param url Url.
    void OpenUrlRequest(const QUrl &url, bool activateNewTab);

    /// Request handled by 3rd party ui logic.
    void ShowProgressScreenRequest(QString message);
    
    /// Request handled by 3rd party ui logic.
    void HideProgressScreenRequest();
    
    /// Request handled by 3rd party ui logic.
    void UpdateProgressScreenRequest(QString message, int progress);

    /// Request handled by 3rd party ui logic.
    /// @note Maximum image size is 300x100 px.
    void UpdateProgressImageRequest(QImage image);

    /// Requests handled by 3rd party ui logic.
    /// Hides/shows the browser ui.
    void SetBrowserVisibilityRequest(bool visible);

private:
    /// Main cookie jar that can be shared across Tundra functionality.
    QPointer<CookieJar> mainCookieJar_;

    /// Main disk cache that can be shared across Tundra functionality.
    QPointer<QNetworkDiskCache> mainDiskCache_;

    QString dataDir_;
    QString reserverCookieFile_;
    QString reservedCookiePath_;
};