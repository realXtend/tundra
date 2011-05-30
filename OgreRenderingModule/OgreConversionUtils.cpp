/** 
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreConversionUtils.cpp
 *  @brief  Contains some common methods for conversions between Ogre and Core variable types.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CoreStringUtils.h"
#include "OgreConversionUtils.h"
#include "MemoryLeakCheck.h"

namespace OgreRenderer
{

Ogre::ColourValue ToOgreColor(const Color& color)
{
    return Ogre::ColourValue(color.r, color.g, color.b, color.a);
}

Color ToCoreColor(const Ogre::ColourValue& color)
{
   return Color(color.r, color.g, color.b, color.a);
}

Ogre::Vector3 ToOgreVector3(const Vector3df &vector)
{
    return Ogre::Vector3(vector.x, vector.y, vector.z);
}

Vector3df ToCoreVector(const Ogre::Vector3 &vector)
{
    return Vector3df(vector.x, vector.y, vector.z);
}

Ogre::Quaternion ToOgreQuaternion(const Quaternion &quat)
{
    return Ogre::Quaternion(quat.w, quat.x, quat.y, quat.z);
}

std::string SanitateAssetIdForOgre(const QString& input)
{
    QString ret = input;
    if (ret.contains('$'))
        return ret.toStdString();

    ret.replace(':', "$1");
    ret.replace('/', "$2");
    return ret.toStdString();
}

std::string SanitateAssetIdForOgre(const std::string& input)
{
    return SanitateAssetIdForOgre(QString::fromStdString(input));
}

std::string SanitateAssetIdForOgre(const char* input)
{
    if (!input)
        return std::string();
    return SanitateAssetIdForOgre(std::string(input));
}

QString DesanitateAssetIdFromOgre(const QString &input)
{
    QString ret = input;
    ret.replace("$1", ":");
    ret.replace("$2", "/");
    return ret;
}

QString DesanitateAssetIdFromOgre(const std::string &input)
{
    return DesanitateAssetIdFromOgre(QString::fromStdString(input));
}

std::string AddDoubleQuotesIfNecessary(const std::string &str)
{
    std::string ret = str;
    size_t found = ret.find(' ');
    if (found != std::string::npos)
    {
        ret.insert(0, "\"");
        ret.append("\"");
    }

    return ret;
}

void DesanitateAssetIds(std::string &script, const QStringList &keywords)
{
    QStringList lines = QString(script.c_str()).split("\n");
    for(int i = 0; i < lines.size(); ++i)
    {
        QString id;
        int idx = -1, offset = -1;
        foreach(const QString &keyword, keywords)
            if (lines[i].contains(keyword))
            {
                idx = lines[i].indexOf(keyword);
                offset = keyword.length();
                id = keyword;
                break;
            }

        if (idx != -1 && offset != -1)
        {
            QString desanitatedRef = DesanitateAssetIdFromOgre(lines[i].mid(idx + offset).trimmed());
            lines[i] = lines[i].left(idx);
            lines[i].append(id + desanitatedRef);
        }
    }

    script = lines.join("\n").toStdString();
}

}
