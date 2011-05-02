// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "VersionInfo.h"

// ApiVersionInfo

ApiVersionInfo::ApiVersionInfo(QObject *parent, uint major, uint minor, uint majorpatch, uint minorpatch) :
    QObject(parent)
{
    versionNumbers_.push_back(major);
    versionNumbers_.push_back(minor);
    versionNumbers_.push_back(majorpatch);
    versionNumbers_.push_back(minorpatch);
}

ApiVersionInfo::~ApiVersionInfo()
{
    versionNumbers_.clear();
}

QString ApiVersionInfo::toString()
{
    QString versionString;
    std::vector<uint>::iterator itr;
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

std::vector<uint> ApiVersionInfo::GetNumbers()
{
    return versionNumbers_;
}

uint ApiVersionInfo::GetMajor()
{
    return versionNumbers_[0];
}

uint ApiVersionInfo::GetMinor()
{
    return versionNumbers_[1];
}

uint ApiVersionInfo::GetMajorPatch()
{
    return versionNumbers_[2];
}

uint ApiVersionInfo::GetMinorPatch()
{
    return versionNumbers_[3];
}

// ApplicationVersionInfo

ApplicationVersionInfo::ApplicationVersionInfo(QObject *parent, uint major, uint minor, uint majorpatch, uint minorpatch, const QString &organization, const QString &name) :
    QObject(parent),
    name_(name),
    organization_(organization)
{
    versionNumbers_.push_back(major);
    versionNumbers_.push_back(minor);
    versionNumbers_.push_back(majorpatch);
    versionNumbers_.push_back(minorpatch);
}

ApplicationVersionInfo::~ApplicationVersionInfo()
{
    versionNumbers_.clear();
}

QString ApplicationVersionInfo::toString()
{
    QStringList components;
    components << GetOrganization() << GetName() << GetVersionString();
    return components.join(" ");
}

QString ApplicationVersionInfo::GetOrganization()
{
    return organization_;
}

QString ApplicationVersionInfo::GetName()
{
    return name_;
}

QString ApplicationVersionInfo::GetVersion()
{
    return GetVersionString();
}

std::vector<uint> ApplicationVersionInfo::GetNumbers()
{
    return versionNumbers_;
}

uint ApplicationVersionInfo::GetMajor()
{
    return versionNumbers_[0];
}

uint ApplicationVersionInfo::GetMinor()
{
    return versionNumbers_[1];
}

uint ApplicationVersionInfo::GetMajorPatch()
{
    return versionNumbers_[2];
}

uint ApplicationVersionInfo::GetMinorPatch()
{
    return versionNumbers_[3];
}

QString ApplicationVersionInfo::GetVersionString()
{
    QString versionString;
    std::vector<uint>::iterator itr;
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
