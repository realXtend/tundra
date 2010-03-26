// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "Md5.h"

#include <QString>
#include <QCryptographicHash>

namespace ProtocolUtilities
{

std::string GetMd5Hash(const std::string &str)
{
    QCryptographicHash engine(QCryptographicHash::Md5);
    engine.addData(str.c_str(), str.size());
    return QString(engine.result().toHex()).toStdString();
}

}

