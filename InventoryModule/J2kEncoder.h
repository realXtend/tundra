// For conditions of distribution and use, see copyright notice in license.txt

/// @file J2kEncoder.h
/// @brief J2k encoding functionality required by image uploads.

#ifndef incl_InventoryModule_J2kEncoder_h
#define incl_InventoryModule_J2kEncoder_h

#include <OgreImage.h>
#include "CoreTypes.h"

namespace J2k
{
    /// J2k error print callback.
    void J2kErrorCallback(const char *msg, void *);

    /// J2k warning print callback.
    void J2kWarningCallback(const char *msg, void *);

    /// J2k info print callback.
    void J2kInfoCallback(const char *msg, void *);

    /// @return Is the value power of two.
    bool IsPowerOfTwo(int value);

    /// @return Closest power of two value.
    int GetClosestPowerOfTwo(int value);

    /// J2k encoding function.
    /// @param src_image
    /// @param outbuf
    /// @param reversible
    bool J2kEncode(Ogre::Image &src_image, std::vector<u8> &outbuf, bool reversible);
}

#endif
