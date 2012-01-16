// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "VersionInfo.h"
#include "LoggingFunctions.h"

// ApiVersionInfo
ApiVersionInfo::ApiVersionInfo(const char *version)
{
    QStringList numbers = QString(version).split('.');
    if (numbers.size() > 4)
        LogError("ApiVersionInfo: More than 4 numbers given. Ignoring extra numbers.");
    for(int i = 0; i < numbers.size() && i < 4; ++i)
    {
        bool ok;
        versionNumbers_.push_back(numbers[i].toInt(&ok));
        assert(ok);
    }
    // Guarantee trailing zeros.
    while(versionNumbers_.size() < 4)
        versionNumbers_.push_back(0);
}

ApiVersionInfo::~ApiVersionInfo()
{
}

std::vector<uint> ApiVersionInfo::GetNumbers() const
{
    return versionNumbers_;
}

QString ApiVersionInfo::GetFullIdentifier() const
{
    QString versionString;
    std::vector<uint>::const_iterator itr;
    for (itr = versionNumbers_.begin(); itr < versionNumbers_.end(); ++itr)
    {
        versionString += QString::number(*itr);
        if (itr < versionNumbers_.end())
            versionString += ".";
    }

    if (versionString.endsWith("."))
        versionString.chop(1);
    if (versionString.endsWith(".0.0"))
        versionString.chop(4);
    else if (versionString.endsWith(".0"))
        versionString.chop(2);
    return versionString;
}

uint ApiVersionInfo::GetMajor() const
{
    return versionNumbers_[0];
}

uint ApiVersionInfo::GetMinor() const
{
    return versionNumbers_[1];
}

uint ApiVersionInfo::GetMajorPatch() const
{
    return versionNumbers_[2];
}

uint ApiVersionInfo::GetMinorPatch() const
{
    return versionNumbers_[3];
}

// ApplicationVersionInfo

ApplicationVersionInfo::ApplicationVersionInfo(const char *organization, const char *name, const char *version) :
    organization_(organization),
    name_(name)
{
    QStringList numbers = QString(version).split('.');
    if (numbers.size() > 4)
        LogError("ApiVersionInfo: More than 4 numbers given. Ignoring extra numbers.");
    for(int i = 0; i < numbers.size() && i < 4; ++i)
    {
        bool ok;
        versionNumbers_.push_back(numbers[i].toInt(&ok));
        assert(ok);
    }
    // Guarantee trailing zeros.
    while(versionNumbers_.size() < 4)
        versionNumbers_.push_back(0);
}

ApplicationVersionInfo::~ApplicationVersionInfo()
{
}

std::vector<uint> ApplicationVersionInfo::GetNumbers() const
{
    return versionNumbers_;
}

QString ApplicationVersionInfo::GetFullIdentifier() const
{
    QStringList components;
    components << GetOrganization() << GetName() << GetVersion();
    return components.join(" ");
}

QString ApplicationVersionInfo::GetOrganization() const
{
    return organization_;
}

QString ApplicationVersionInfo::GetName() const
{
    return name_;
}

QString ApplicationVersionInfo::GetVersion() const
{
    QString versionString;
    std::vector<uint>::const_iterator itr;
    for (itr = versionNumbers_.begin(); itr < versionNumbers_.end(); ++itr)
    {
        versionString += QString::number(*itr);
        if (itr < versionNumbers_.end())
            versionString += ".";
    }

    if (versionString.endsWith("."))
        versionString.chop(1);
    if (versionString.endsWith(".0.0"))
        versionString.chop(4);
    else if (versionString.endsWith(".0"))
        versionString.chop(2);
    return versionString;
}

uint ApplicationVersionInfo::GetMajor() const
{
    return versionNumbers_[0];
}

uint ApplicationVersionInfo::GetMinor() const
{
    return versionNumbers_[1];
}

uint ApplicationVersionInfo::GetMajorPatch() const
{
    return versionNumbers_[2];
}

uint ApplicationVersionInfo::GetMinorPatch() const
{
    return versionNumbers_[3];
}
