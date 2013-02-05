// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "CoreTypes.h"

#include <QObject>

/// Utility class for retrieving versioning information.
/** @sa Application::OrganizationName, Application::ApplicationName, Application::Version, Application::FullIdentifier */
class TUNDRACORE_API VersionInfo : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString version READ Version) ///< @copydoc Version
    Q_PROPERTY(uint major READ Major) ///< @copydoc Major
    Q_PROPERTY(uint minor READ Minor) ///< @copydoc Minor
    Q_PROPERTY(uint majorPatch READ MajorPatch) ///< @copydoc MajorPatch
    Q_PROPERTY(uint minorPatch READ MinorPatch) ///< @copydoc MinorPatch

public:
    /// Constucts version information from string.
    /** @param version Version information e.g. "2.0.0.0" */
    explicit VersionInfo(const char *version);

    /// Returns the full version indentifier, omitting trailing zeros.
    QString Version() const;

    /// Returns the major version.
    uint Major() const { return numbers[0]; }

    /// Returns the minor version.
    uint Minor() const { return numbers[1]; }

    /// Returns the major patch version.
    uint MajorPatch() const { return numbers[2]; }

    /// Returns the minor patch version.
    uint MinorPatch() const { return numbers[3]; }

private:
    std::vector<uint> numbers;
};
