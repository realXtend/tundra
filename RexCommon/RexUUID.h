// For conditions of distribution and use, see copyright notice in license.txt
#ifndef incl_RexUUID_h
#define incl_RexUUID_h

#include "RexTypes.h"

namespace RexTypes
{

    /// RexUUID is a 16-byte identifier for resources in a virtual world.
    class RexUUID
    {
    public:
        /// Constructs an RexUUID from a string in form "1c1bbda2-304b-4cbf-ba3f-75324b044c73" or "1c1bbda2304b4cbfba3f75324b044c73".
        explicit RexUUID(const char *str);
        explicit RexUUID(const std::string &str);
    
        /// Constructs a null RexUUID.
        RexUUID();

        /// Sets all 16 bytes of the ID to '00'.
        void SetNull();
        
        /// Random-generates the contents
        void Random();
    
        void FromString(const char *str);
        void FromString(const std::string &str) { FromString(str.c_str()); }

        std::string ToString() const;
    
        bool operator ==(const RexUUID &rhs) const;
        bool operator <(const RexUUID &rhs) const;

        static const uint8_t cSizeBytes = 16;

        uint8_t data[cSizeBytes];
    };

}

#endif
