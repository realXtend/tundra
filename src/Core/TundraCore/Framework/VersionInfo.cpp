// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "VersionInfo.h"
#include "LoggingFunctions.h"

VersionInfo::VersionInfo(const char *version)
{
    QStringList numberList = QString(version).split('.');
    if (numberList.size() > 4)
        LogWarning("VersionInfo: More than 4 numbers given. Ignoring extra numbers.");
    for(int i = 0; i < numberList.size() && i < 4; ++i)
    {
        int versionNumber = 0;
        sscanf(numberList[i].toStdString().c_str(), "%d", &versionNumber); // If fails, push 0.
        numbers.push_back(versionNumber);
    }
    // Guarantee trailing zeros.
    while(numbers.size() < 4)
        numbers.push_back(0);
}

QString VersionInfo::Version() const
{
    QString str;
    for(std::vector<uint>::const_iterator it = numbers.begin(); it < numbers.end(); ++it)
    {
        str += QString::number(*it);
        if (it < numbers.end())
            str += ".";
    }
    if (str.endsWith("."))
        str.chop(1);
    if (str.endsWith(".0.0"))
        str.chop(4);
    else if (str.endsWith(".0"))
        str.chop(2);
    return str;
}
