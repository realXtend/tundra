// For conditions of distribution and use, see copyright notice in license.txt

#include <QNetworkCookieJar>

namespace CoreUi
{
    namespace Classical
    {
        class WebLoginPermanentCookie : public QNetworkCookieJar
        {

        Q_OBJECT

        public:
            WebLoginPermanentCookie(QObject* parent);
            ~WebLoginPermanentCookie();

            void loadFromDisk();
            void flushToDisk() const;

        public slots:
            void clear();
        private:
            QString cookie_fileName;
        };

    }
}

