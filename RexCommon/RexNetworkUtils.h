// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexNetworkUtils_h
#define incl_RexNetworkUtils_h

#include "RexTypes.h"
#include "RexUUID.h"
#include "Color.h"
#include "Quaternion.h"
#include "Vector3D.h"

namespace RexTypes
{       
    //! Helper functions for getting values from terseupdate packets
    Quaternion GetProcessedQuaternion(const uint8_t* bytes);
    Vector3df GetProcessedScaledVectorFromUint16(const uint8_t* bytes, float scale);
    Vector3df GetProcessedVectorFromUint16(const uint8_t* bytes);
    Vector3df GetProcessedVector(const uint8_t* bytes);

    bool IsValidPositionVector(const Vector3df &pos);
    bool IsValidVelocityVector(const Vector3df &pos);

    bool ParseBool(const std::string &value);
    
    // Helper functions for reading values from byteblob:

    //! \todo Remove, not serialization-safe. (bool assumed 8bit.)
    bool ReadBoolFromBytes(const uint8_t* bytes, int& idx);

    uint8_t ReadUInt8FromBytes(const uint8_t* bytes, int& idx);
    uint16_t ReadUInt16FromBytes(const uint8_t* bytes, int& idx);
    uint32_t ReadUInt32FromBytes(const uint8_t* bytes, int& idx);
    int16_t ReadSInt16FromBytes(const uint8_t* bytes, int& idx);
    int32_t ReadSInt32FromBytes(const uint8_t* bytes, int& idx);
    float ReadFloatFromBytes(const uint8_t* bytes, int& idx);
    Color ReadColorFromBytes(const uint8_t* bytes, int& idx);
    Color ReadColorFromBytesInverted(const uint8_t* bytes, int& idx);
    RexUUID ReadUUIDFromBytes(const uint8_t* bytes, int& idx);
    std::string ReadNullTerminatedStringFromBytes(const uint8_t* bytes, int& idx);
    bool ReadTextureEntryBits(uint32_t& bits, int& num_bits, const uint8_t* bytes, int& idx);
    
    //! Helper functions for writing values into byteblob
    void WriteBoolToBytes(bool value, uint8_t* bytes, int& idx);
    void WriteUInt8ToBytes(uint8_t value, uint8_t* bytes, int& idx);
    void WriteUInt16ToBytes(uint16_t value, uint8_t* bytes, int& idx);    
    void WriteUInt32ToBytes(uint32_t value, uint8_t* bytes, int& idx);
    void WriteFloatToBytes(float value, uint8_t* bytes, int& idx);
    void WriteUUIDToBytes(const RexUUID &value, uint8_t* bytes, int& idx);    
    void WriteNullTerminatedStringToBytes(const std::string& value, uint8_t* bytes, int& idx);
    
    //! Helper function for parsing namevaluemap
    typedef std::map<std::string, std::string> NameValueMap;
    NameValueMap ParseNameValueMap(const std::string& namevalue);
}

#endif