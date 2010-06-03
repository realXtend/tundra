/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   RexUUID.h
 *  @brief  RexUUID is a 16-byte identifier for resources in a virtual world.
*/

#ifndef incl_RexCommon_RexUUID_h
#define incl_RexCommon_RexUUID_h

#include "RexTypes.h"

#include <QDebug>

/// RexUUID is a 16-byte identifier for resources in a virtual world.
class RexUUID
{
public:
    /// Default constuctor. Constructs a null RexUUID.
    RexUUID();

    /// Constructor. Constructs an RexUUID from a string in form "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
    /// @param str String.
    explicit RexUUID(const char *str);
    explicit RexUUID(const std::string &str);
    explicit RexUUID(const QString &str);

    /// Sets all 16 bytes of the ID to '00'.
    void SetNull();

    /// Checks is the RexUUID null.
    bool IsNull() const;

    /// Random-generates the contents
    void Random();

    /// Creates a random RexUUID.
    static RexUUID CreateRandom();

    /// Creates an RexUUID from string.
    /// @param str String.
    void FromString(const char *str);
    void FromString(const std::string &str) { FromString(str.c_str()); }
    void FromString(const QString &str) { FromString(str.toStdString()); }

    /// Converts the RexUUID to a QString.
    std::string ToString() const;

    /// Converts the RexUUID to a QString.
    QString ToQString() const { return ToString().c_str(); }

    /// Tests whether a string contains a valid UUID
    /// @param str String.
    static bool IsValid(const char *str);
    static bool IsValid(const std::string &str) { return IsValid(str.c_str()); }
    static bool IsValid(const QString &str) { return IsValid(str.toStdString()); }

    /// Assign operator. Copies data from rhs to the new RexUUID.
    RexUUID &operator =(const RexUUID &rhs);

    /// Equality operator
    /// @return true if the RexUUID's are indentical, false otherwise.
    bool operator ==(const RexUUID &rhs) const;

    /// Inequality operator
    /// @return true if the RexUUID's don't match, false if they do.
    bool operator !=(const RexUUID &rhs) const;

    /// Less than operator
    /// @return true if the left-hand side is less than the right-hand side.
    bool operator <(const RexUUID &rhs) const;

    /// Greater than operator
    /// @return true if the left-hand side is greater than the right-hand side.
    bool operator >(const RexUUID &rhs) const;

    /// Serializes RexUUID to ostream
    friend std::ostream& operator << (std::ostream &out, const RexUUID &rhs)
    {
        out << rhs.ToString();
        return out;
    }

    /// Serializes RexUUID to QDebug
    friend QDebug &operator << (QDebug &dbg, const RexUUID &rhs)
    {
        dbg.nospace() << rhs.ToQString();
        return dbg.space();
    }

    /// Size in bytes.
    static const uint8_t cSizeBytes = 16;

    /// Data.
    uint8_t data[cSizeBytes];
};

#endif
