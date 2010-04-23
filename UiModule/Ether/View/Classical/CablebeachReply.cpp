#include "StableHeaders.h"
#include <QtNetwork>
#include "CablebeachReply.h"

CablebeachReply::CablebeachReply(const QUrl &url) : QNetworkReply()
{
    content = "<html><head><title>Cablebeach reply</title></head><body>Processing cablebeach url</body></html>";
    offset = 0;

    setHeader(QNetworkRequest.ContentTypeHeader, QVariant("text/html; charset=ASCII"));
    setHeader(QNetworkRequest.ContentLengthHeader, QVariant(content.size()));

    emit readyRead();
    emit finished();
    open(ReadOnly | Unbuffered);
    setUrl(url);
}

void CablebeachReply::abort()
{
}

qint64 CablebeachReply::bytesAvailable() const
{
    return content.size() - offset;
}

bool CablebeachReply::isSequential() const
{
    return true;
}

qint64 CablebeachReply::readData(char *data, qint64 maxSize)
{
    if (offset < content.size())
    {
        qint64 end = qMin(maxSize, content.size() - offset);
        memcpy(data, content.constData() + offset, end);
        offset += end;
        return end;
    }
    else
        return -1;
}
