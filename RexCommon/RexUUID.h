// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   RexUUID.h
 *  @brief  RexUUID is a 16-byte identifier for resources in a virtual world.
*/

#ifndef incl_RexUUID_h
#define incl_RexUUID_h

#include "RexTypes.h"

class RexUUID
{
public:
    /// Constructs an RexUUID from a string in form "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
    /// @param str String.
    explicit RexUUID(const char *str);
    explicit RexUUID(const std::string &str);

    /// Constructs a null RexUUID.
    RexUUID();

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

    /// Converts the RexUUID to a string.
    /// @return RexUUID as a string.
    std::string ToString() const;

    /// Tests whether a string contains a valid UUID
    /// @param str String.
    static bool IsValid(const char *str);
    static bool IsValid(const std::string &str) { return IsValid(str.c_str()); }

    RexUUID &operator =(const RexUUID &rhs);

    bool operator ==(const RexUUID &rhs) const;

    bool operator !=(const RexUUID &rhs) const;

    bool operator <(const RexUUID &rhs) const;

    friend std::ostream& operator << ( std::ostream &out, const RexUUID &r )
    {
        out << "RexUUID(" << r.ToString() << ")";
        return out;
    }

    /// Size in bytes.
    static const uint8_t cSizeBytes = 16;

    /// Data.
    uint8_t data[cSizeBytes];
};

#endif
