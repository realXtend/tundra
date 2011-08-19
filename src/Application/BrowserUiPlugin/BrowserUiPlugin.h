// For conditions of distribution and use, see copyright notice in license.txt

#include "FrameworkFwd.h"

#include <QObject>
#include <QString>
#include <QAction>
#include <QUrl>

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

    /// Creates a new cookie jar that implements disk writing and reading. Can be used with any QNetworkAccessManager with setCookieJar() function.
    /// \note AssetCache will be the CookieJars parent and it will destroyed by it, don't take ownerwhip of the returned CookieJar.
    /// \param QString File path to the file the jar will read/write cookies to/from.
    CookieJar *CreateCookieJar(const QString &cookieDiskFile);

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

private:
    /// Framework ptr.
    Framework *framework_;

};