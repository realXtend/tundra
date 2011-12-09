// For conditions of distribution and use, see copyright notice in LICENSE

#include "CookieJar.h"
#include <QFile>

CookieJar::CookieJar(QObject *parent, const QString &cookieDiskFile) :
    QNetworkCookieJar(parent),
    cookieDiskFile_(cookieDiskFile)
{
    ReadCookies();
}

CookieJar::~CookieJar()
{
    StoreCookies();
}

void CookieJar::SetDataFile(const QString &cookieDiskFile)
{
    cookieDiskFile_ = cookieDiskFile;
}

void CookieJar::ClearCookies()
{
    if (!cookieDiskFile_.isEmpty())
    {
        QFile cookiesFile(cookieDiskFile_);
        if (cookiesFile.exists())
            cookiesFile.remove();
    }
    setAllCookies(QList<QNetworkCookie>());
}

void CookieJar::ReadCookies()
{
    if (cookieDiskFile_.isEmpty())
        return;

    QFile cookiesFile(cookieDiskFile_);
    if (!cookiesFile.open(QIODevice::ReadOnly))
        return;

    QList<QNetworkCookie> cookies;
    QDataStream cookieData(&cookiesFile);
    while (!cookieData.atEnd()) 
    {
        QByteArray rawCookie;
        cookieData >> rawCookie;
        cookies.append(QNetworkCookie::parseCookies(rawCookie));
    }
    cookiesFile.close();
    setAllCookies(cookies);
}

void CookieJar::StoreCookies()
{
    if (cookieDiskFile_.isEmpty())
        return;
    if (allCookies().empty())
        return;

    QFile cookiesFile(cookieDiskFile_);
    if (!cookiesFile.open(QIODevice::WriteOnly))
        return;

    QDataStream cookieData(&cookiesFile);
    foreach (QNetworkCookie cookie, allCookies())
        cookieData << cookie.toRawForm();
    cookiesFile.close();
}
