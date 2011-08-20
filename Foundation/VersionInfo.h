/// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_VersionInfo_h
#define incl_Foundation_VersionInfo_h

#include "StableHeaders.h"

#include <QObject>
#include <QString>

/*! \brief ApiVersionInfo is for getting Tundra API version information.
    \details This may be helpful for scripts to check the API version. 
    You might detect that some part of your script wont run with certain versions and you 
    can possibly go around the restriction in the api.

    JavaScript example:
    \code
    print(apiversion.toString());
    if (apiversion.major < 1)
        print("Note: API in not yet at stable version");
    etc.
    \endcode
*/
class ApiVersionInfo : public QObject
{

Q_OBJECT
Q_PROPERTY(uint major READ GetMajor)
Q_PROPERTY(uint minor READ GetMinor)
Q_PROPERTY(uint majorpatch READ GetMajorPatch)
Q_PROPERTY(uint minorpatch READ GetMinorPatch)

public:
    /// Constructor.
    explicit ApiVersionInfo(QObject *parent, uint major, uint minor, uint majorpatch, uint minorpatch);

    /// Destructor.
    ~ApiVersionInfo();

public slots:
    /// \todo Add comparison support.

    /// Returns a string of the Tundra API version.
    /// \note This function will omit .0 and .0.0 from the end of the string.
    /// \return QString Tundra API version.
    QString toString();

    /// Get Tundra API major version.
    /// \return uint Tundra API major version.
    uint GetMajor();

    /// Get Tundra API minor version.
    /// \return uint Tundra API minor version.
    uint GetMinor();

    /// Get Tundra API major patch version.
    /// \return uint Tundra API major patch version.
    uint GetMajorPatch();

    /// Get Tundra API minor patch version.
    /// \return uint Tundra API minor patch version.
    uint GetMinorPatch();

    /// Get all version numbers as a std vector.
    /// \return std::vector<uint> All Tundra API version numbers.
    std::vector<uint> GetNumbers();

private:
    std::vector<uint> versionNumbers_;
};

/*! \brief ApplicationVersionInfo is for getting Tundra application version information.
    \details This is helpful all around the app when you need version shown to user in ui. Or detect application version numbers for some reason.
    The toString() function is for example used in our main windows title.

    \note If you want to detect API version instead of application version please see ApiVersionInfo class.

    JavaScript example:
    \code
    var appIdLabel = new QLabel(applicationversion.toString());
    etc.
    \endcode
*/
class ApplicationVersionInfo : public QObject
{

Q_OBJECT
Q_PROPERTY(uint major READ GetMajor)
Q_PROPERTY(uint minor READ GetMinor)
Q_PROPERTY(uint majorpatch READ GetMajorPatch)
Q_PROPERTY(uint minorpatch READ GetMinorPatch)
Q_PROPERTY(QString organization READ GetOrganization)
Q_PROPERTY(QString name READ GetName)

public:
    /// Constructor.
    explicit ApplicationVersionInfo(QObject *parent, uint major, uint minor, uint majorpatch, uint minorpatch, const QString &organization, const QString &name);

    /// Destructor.
    ~ApplicationVersionInfo();

public slots:
    /// Returns a full identifier of the Tundra application as "<organization> <name> <full_version>" eg. "realXtend Tundra 1.0.6".
    /// \note This function will omit .0 and .0.0 from the end of the version string.
    /// \return QString Full Tundra application identifier.
    QString toString();

    /// Get the Tundra applications organization, eg. "realXtend".
    /// \return QString Tundra applications organization.
    QString GetOrganization();

    /// Get Tundra application name, eg. "Tundra".
    /// \return QString Tundra application name.
    QString GetName();

    /// Get Tundra application version, eg. "1.0.6".
    /// \note This function will omit .0 and .0.0 from the end of the version string.
    /// \return QString 
    QString GetVersion();

    /// Get Tundra application major version.
    /// \return uint Tundra application major version.
    uint GetMajor();

    /// Get Tundra application minor version.
    /// \return uint Tundra application minor version.
    uint GetMinor();

    /// Get Tundra application major patch version.
    /// \return uint Tundra application major patch version.
    uint GetMajorPatch();

    /// Get Tundra application minor patch version.
    /// \return uint Tundra application minor patch version.
    uint GetMinorPatch();

    /// Get all version numbers as a std vector.
    /// \return std::vector<uint> All Tundra application version numbers.
    std::vector<uint> GetNumbers();

private slots:
    QString GetVersionString();

private:
    std::vector<uint> versionNumbers_;
    QString name_;
    QString organization_;
};


#endif
