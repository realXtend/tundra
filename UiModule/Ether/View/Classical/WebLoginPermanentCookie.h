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

