#include "StableHeaders.h"
#include "WebLoginPermanentCookie.h"
namespace CoreUi
{
    namespace Classical
    {

        WebLoginPermanentCookie::WebLoginPermanentCookie(QObject* parent = 0)
            : QNetworkCookieJar(parent),
            cookie_fileName(QDir::toNativeSeparators(QDir::homePath()) + QDir::separator() + ".naali-cookies")
        {
            loadFromDisk();
        }

        WebLoginPermanentCookie::~WebLoginPermanentCookie()
        {
            flushToDisk();
        }

        void WebLoginPermanentCookie::clear()
        {
            QFile cookiesFile(cookie_fileName);
            if (cookiesFile.exists())
            {
                cookiesFile.remove();
            }
            setAllCookies(QList<QNetworkCookie>());
        }

        void WebLoginPermanentCookie::loadFromDisk()
        {
            // open the file and open a data stream over it
            QFile cookiesFile(cookie_fileName);
            if (!cookiesFile.open(QIODevice::ReadOnly)) {
                qWarning("WebLoginPermanentCookie::loadFromDisk: error loading cookies from '%s'", qPrintable(cookie_fileName));
                return;
            }
            QDataStream cs(&cookiesFile);

            // read all cookies
            QList<QNetworkCookie> cookieList;
            while (!cs.atEnd()) {
                QByteArray rawCookie;
                cs >> rawCookie;
                cookieList.append(QNetworkCookie::parseCookies(rawCookie));
            }

            // set the internal cookie list
            setAllCookies(cookieList);
        }

        void WebLoginPermanentCookie::flushToDisk() const
        {
            // open the file and open a data stream over it
            QFile cookiesFile(cookie_fileName);
            if (!cookiesFile.open(QIODevice::WriteOnly)) {
                qWarning("WebLoginPermanentCookie::flushToDisk: error saving cookies to '%s'", qPrintable(cookie_fileName));
                return;
            }
            QDataStream cs(&cookiesFile);

            // write all cookies
            foreach (QNetworkCookie cookie, allCookies())
                cs << cookie.toRawForm();
        }

    }
}

