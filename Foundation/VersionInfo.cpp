#include "StableHeaders.h"
#include "VersionInfo.h"
#include <QString>

// For conditions of distribution and use, see copyright notice in license.txt
VersionInfo::VersionInfo(uint major, uint minor, uint majorpatch, uint minorpatch)
{
    versionnumbers_.push_back(major);
    versionnumbers_.push_back(minor);
    versionnumbers_.push_back(majorpatch);
    versionnumbers_.push_back(minorpatch);
}

VersionInfo::~VersionInfo()
{
}

uint VersionInfo::GetMajor()
{
    return versionnumbers_[0];
}

uint VersionInfo::GetMinor()
{
    return versionnumbers_[1];
}

uint VersionInfo::GetMajorPatch()
{
    return versionnumbers_[2];
}

uint VersionInfo::GetMinorPatch()
{
    return versionnumbers_[3];
}

QString VersionInfo::toString()
{
    QString versionstring;

    std::vector<uint>::iterator itr;
	for ( itr = versionnumbers_.begin(); itr < versionnumbers_.end(); ++itr )
    {
        versionstring += QString::number(*itr);
        if (itr < versionnumbers_.end())
            versionstring += ".";
    }

    //todo: omit trailing 0s

    return versionstring;
}
