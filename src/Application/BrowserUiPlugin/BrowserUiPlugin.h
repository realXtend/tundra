// For conditions of distribution and use, see copyright notice in license.txt

#include "FrameworkFwd.h"

#include <QObject>
#include <QString>
#include <QAction>
#include <QUrl>
#include <QImage>

class CookieJar;
class QScriptEngine;

class BrowserUiPlugin : public QObject
{

Q_OBJECT

public:
    /// Constructor.
    BrowserUiPlugin(Framework *framework);

    /// Deconstructor
    ~BrowserUiPlugin();

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

    /// Returns the main cookie jar that can be shared across Tundra functionality.
    /// Can be used with any QNetworkAccessManager with setCookieJar() function.
    /// @return CookieJar* Main cookie jar.
    CookieJar *MainCookieJar();

    /// Creates a new cookie jar that implements disk writing and reading. Can be used with any QNetworkAccessManager with setCookieJar() function.
    /// @note AssetCache will be the CookieJars parent and it will destroyed by it, don't take ownership of the returned CookieJar.
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

private:
    /// Framework ptr.
    Framework *framework_;

    /// Main cookie jar that can be shared across Tundra functionality.
    CookieJar *mainCookieJar_;

    QString reserverCookieFile_;
    QString reservedCookiePath_;
};