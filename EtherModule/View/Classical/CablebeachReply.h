#include <QNetworkReply>
#include <QUrl>

class CablebeachReply : public QNetworkReply
{
    Q_OBJECT

public:
    CablebeachReply(const QUrl &url);
    void abort();
    qint64 bytesAvailable() const;
    bool isSequential() const;

protected:
    qint64 readData(char *data, qint64 maxSize);

private:
    QByteArray content;
    qint64 offset;
};
