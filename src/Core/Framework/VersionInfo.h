// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "CoreTypes.h"

#include <QObject>
#include <QString>

/// Tundra API version information.
/** This may be helpful for scripts to check the API version.
    You might detect that some part of your script wont run with certain versions and you
    can possibly go around the restriction in the api.

    JavaScript example:
    @code
    print(apiversion.GetFullIdentifier());
    if (apiversion.major < 1)
        print("Note: API in not yet at stable version");
    etc.
    @endcode

    @todo Consider not storing version information as uints - what if we want to have e.g. "2.3beta" etc.? */
class ApiVersionInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint major READ GetMajor)
    Q_PROPERTY(uint minor READ GetMinor)
    Q_PROPERTY(uint majorPatch READ GetMajorPatch)
    Q_PROPERTY(uint minorPatch READ GetMinorPatch)

public:
    /// Constucts version information from string.
    /** @param version Version information e.g. "2.0.0.0" */
    explicit ApiVersionInfo(const char *version);
    ~ApiVersionInfo();

    /// Get all version numbers as a std vector.
    std::vector<uint> GetNumbers() const;

public slots:
    /// @todo Add comparison support.

    /// Returns a string of the Tundra API version.
    /// @note This function will omit .0 and .0.0 from the end of the string.
    QString GetFullIdentifier() const;

    /// Get Tundra API major version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMajor() const;

    /// Get Tundra API minor version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMinor() const;

    /// Get Tundra API major patch version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMajorPatch() const;

    /// Get Tundra API minor patch version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMinorPatch() const;

private:
    std::vector<uint> versionNumbers_;
};

/// Tundra application version information.
/** This is helpful all around the app when you need version shown to user in ui. Or detect application version numbers for some reason.
    The toString() function is for example used in our main windows title.

    @note If you want to detect API version instead of application version please see ApiVersionInfo class.

    JavaScript example:
    @code
    var appIdLabel = new QLabel(applicationversion.GetFullIdentifier());
    etc.
    @endcode

    @todo Consider not storing version information as uints - what if we want to have e.g. "2.3beta" etc.? */
class ApplicationVersionInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(uint major READ GetMajor)
    Q_PROPERTY(uint minor READ GetMinor)
    Q_PROPERTY(uint majorPatch READ GetMajorPatch)
    Q_PROPERTY(uint minorPatch READ GetMinorPatch)
    Q_PROPERTY(QString organization READ GetOrganization)
    Q_PROPERTY(QString name READ GetName)

public:
    /// Constucts version information from string.
    /** @param organization Name of the organization, , e.g. "realXtend"
        @param name Name of the application, e.g. "Tundra"
        @param version Version information e.g. "2.0.0.0" */
    ApplicationVersionInfo(const char *organization, const char *name, const char *version);
    ~ApplicationVersionInfo();

    /// Get all version numbers as a std vector.
    std::vector<uint> GetNumbers() const;

public slots:
    /// Returns a full identifier of the Tundra application as "<organization> <name> <full_version>" eg. "realXtend Tundra 2.0.6".
    /// @note This function will omit .0 and .0.0 from the end of the version string.
/// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    QString GetFullIdentifier() const;

    /// Get the Tundra applications organization, eg. "realXtend".
/// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    QString GetOrganization() const;

    /// Get Tundra application name, eg. "Tundra".
/// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    QString GetName() const;

    /// Get Tundra application version, eg. "2.0.6".
    /// @note This function will omit .0 and .0.0 from the end of the version string.
    QString GetVersion() const;

    /// Get Tundra application major version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMajor() const;

    /// Get Tundra application minor version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMinor() const;

    /// Get Tundra application major patch version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMajorPatch() const;

    /// Get Tundra application minor patch version.
    /// @todo Doesn't need to be slot, exposed as Q_PROPERTY
    uint GetMinorPatch() const;

private:
    std::vector<uint> versionNumbers_;
    QString name_;
    QString organization_;
};
