#pragma once

#include "clb/Core/Types.h"
#include <string>

class RexUUID
{
public:
    static const int cSizeBytes = 16;
    
    RexUUID()
    {
        for (int i = 0; i < cSizeBytes; ++i)
            data[i] = 0;
    }
    
    explicit RexUUID(const u8* buffer)
    {
        for (int i = 0; i < cSizeBytes; ++i)
            data[i] = buffer[i];
    }
    
    bool operator == (const RexUUID& rhs) const
    {
        for (int i = 0; i < cSizeBytes; ++i)
            if (data[i] != rhs.data[i])
                return false;
        return true;
    }
    
    bool operator < (const RexUUID& rhs) const
    {
        for (int i = 0; i < cSizeBytes; ++i)
            if (data[i] < rhs.data[i])
                return true;
            else if (rhs.data[i]  < data[i])
                return false;
        return false;
    }
    
    bool operator > (const RexUUID& rhs) const
    {
        return rhs < *this;
    }
    
    bool operator != (const RexUUID& rhs) const
    {
        return !(*this == rhs);
    }
    
    bool IsNull() const
    {
        bool allNull = true;
        for (int i = 0; i < cSizeBytes; ++i)
        {
            if (data[i] != 0)
            {
                allNull = false;
                break;
            }
        }
        return allNull;
    }
    
    std::string ToString() const;

    std::string ToShortString() const;
    
    void ToBuffer(u8* buffer) const
    {
        for (int i = 0; i < cSizeBytes; ++i)
            buffer[i] = data[i];
    }
    
    u8 data[cSizeBytes];
};