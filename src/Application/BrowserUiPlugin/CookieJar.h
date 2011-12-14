// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "BrowserUiPluginApi.h"

#include <QNetworkCookieJar>
#include <QString>

/*! CookieJar is a subclass of QNetworkCookieJar. This is the only way to do disk storage for cookies with Qt.
    Part reason why this is declared here is because JavaScript cannot do proper inheritance of QNetworkCookieJar to access the protected functions.

    \note You can ask BrowserUiPlugin::CreateCookiJar to make you a new CookieJar.
*/
class BROWSERUI_MODULE_API CookieJar : public QNetworkCookieJar
{

Q_OBJECT

public:
    CookieJar(QObject *parent, const QString &cookieDiskFile = QString());
    ~CookieJar();

public slots:
    void SetDataFile(const QString &cookieDiskFile);
    void ClearCookies();
    void ReadCookies();
    void StoreCookies();

private:
    QString cookieDiskFile_;

};
