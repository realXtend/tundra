/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_VersionInfo_h
#define incl_Foundation_VersionInfo_h

#include "StableHeaders.h"
#include <QObject>

class VersionInfo : public QObject
{
    Q_OBJECT;
    Q_PROPERTY(uint major READ GetMajor);
    Q_PROPERTY(uint minor READ GetMinor);
    Q_PROPERTY(uint majorpatch READ GetMajorPatch);
    Q_PROPERTY(uint minorpatch READ GetMinorPatch);

public slots:
    QString toString();

    //todo: add comparison support

public:
    explicit VersionInfo(uint major, uint minor, uint majorpatch, uint minorpatch);

    //! destructor
    ~VersionInfo();

    //! getters for the individual parts
    uint GetMajor();
    uint GetMinor();
    uint GetMajorPatch();
    uint GetMinorPatch();

    //! all version numbers as a uint[4]
    std::vector<uint> GetNumbers() { return versionnumbers_; }

private:
    std::vector<uint> versionnumbers_;
};

#endif
