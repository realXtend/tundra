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
#include "AssetAPI.h"
#include "MemoryLeakCheck.h"

namespace OgreRenderer
{

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
            QString desanitatedRef = AssetAPI::DesanitateAssetRef(lines[i].mid(idx + offset).trimmed());
            lines[i] = lines[i].left(idx);
            lines[i].append(id + desanitatedRef);
        }
    }

    script = lines.join("\n").toStdString();
}

}
