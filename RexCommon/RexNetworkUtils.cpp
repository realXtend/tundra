// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/algorithm/string.hpp>

#ifndef unix
#include <float.h>
#else
#include "CoreTypes.h"
#endif

#include "RexNetworkUtils.h"
#include "QuatUtils.h"
#include <QStringList>

namespace RexTypes
{    
    bool ParseBool(const std::string &value)
    {
        std::string testedvalue = value;
        boost::algorithm::to_lower(testedvalue);
        return (boost::algorithm::starts_with(testedvalue,"true") || boost::algorithm::starts_with(testedvalue,"1")); 
    }
    
    Quaternion GetProcessedQuaternion(const uint8_t* bytes)
    {    
        uint16_t *rot = reinterpret_cast<uint16_t*>((uint16_t*)&bytes[0]);
        Quaternion rotation = UnpackQuaternionFromU16_4(rot);
        rotation.normalize();
        return rotation;
    }

    Vector3df GetProcessedScaledVectorFromUint16(const uint8_t* bytes, float scale)
    {
        uint16_t *vec = reinterpret_cast<uint16_t*>((uint16_t*)&bytes[0]);
        
        Vector3df resultvector;
        resultvector.x = scale * ((vec[0] / 32768.0f) - 1.0f);
        resultvector.y = scale * ((vec[1] / 32768.0f) - 1.0f);
        resultvector.z = scale * ((vec[2] / 32768.0f) - 1.0f);

        return resultvector;
    }

    Vector3df GetProcessedVectorFromUint16(const uint8_t* bytes)
    {
        uint16_t *vec = reinterpret_cast<uint16_t*>((uint16_t*)&bytes[0]);
        return Vector3df(vec[0],vec[1],vec[2]);
    }

    Vector3df GetProcessedVector(const uint8_t* bytes)
    {
        Vector3df resultvector = *reinterpret_cast<Vector3df*>((Vector3df*)&bytes[0]);
        return resultvector;
    }
    
    bool IsValidPositionVector(const Vector3df &pos)
    {
        // This is a heuristic check to guard against the OpenSim server sending us stupid positions.
        if (fabs(pos.x) > 1e6f || fabs(pos.y) > 1e6f || fabs(pos.z) > 1e6f) 
            return false;
        if (_isnan(pos.x) || _isnan(pos.y) || _isnan(pos.z))
            return false;
        if (!_finite(pos.x) || !_finite(pos.y) || !_finite(pos.z))
            return false;

        return true;
    }

    bool IsValidVelocityVector(const Vector3df &pos)
    {
        // This is a heuristic check to guard against the OpenSim server sending us stupid velocity vectors.
        if (fabs(pos.x) > 1e3f || fabs(pos.y) > 1e3f || fabs(pos.z) > 1e3f)
            return false;
        if (_isnan(pos.x) || _isnan(pos.y) || _isnan(pos.z))
            return false;
        if (!_finite(pos.x) || !_finite(pos.y) || !_finite(pos.z))
            return false;

        return true;
    }

    bool ReadBoolFromBytes(const uint8_t* bytes, int& idx)
    {
        bool result = *(bool*)(&bytes[idx]);
        idx += sizeof(bool);
        
        return result; 
    }
    
    uint8_t ReadUInt8FromBytes(const uint8_t* bytes, int& idx)
    {
        uint8_t result = bytes[idx];
        idx++;
        
        return result;
    }

    uint16_t ReadUInt16FromBytes(const uint8_t* bytes, int& idx)
    {
        uint16_t result = *(uint16_t*)(&bytes[idx]);
        idx += sizeof(uint16_t);
        
        return result;
    }
    
    uint32_t ReadUInt32FromBytes(const uint8_t* bytes, int& idx)
    {
        uint32_t result = *(uint32_t*)(&bytes[idx]);
        idx += sizeof(uint32_t);
        
        return result;
    }

    int16_t ReadSInt16FromBytes(const uint8_t* bytes, int& idx)
    {
        int16_t result = *(int16_t*)(&bytes[idx]);
        idx += sizeof(int16_t);
        
        return result;
    }
    
    int32_t ReadSInt32FromBytes(const uint8_t* bytes, int& idx)
    {
        int32_t result = *(int32_t*)(&bytes[idx]);
        idx += sizeof(int32_t);
        
        return result;
    }
    
    float ReadFloatFromBytes(const uint8_t* bytes, int& idx)
    {
        float result = *(float*)(&bytes[idx]);
        idx += sizeof(float);
        
        return result; 
    }     

    RexUUID ReadUUIDFromBytes(const uint8_t* bytes, int& idx)
    {
        RexUUID result = *(RexUUID*)(&bytes[idx]);
        idx += sizeof(RexUUID);
        
        return result; 
    }
    
    Color ReadColorFromBytes(const uint8_t* bytes, int& idx)
    {
        uint8_t r = bytes[idx++];
        uint8_t g = bytes[idx++];
        uint8_t b = bytes[idx++];
        uint8_t a = bytes[idx++];
        
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
    
    Color ReadColorFromBytesInverted(const uint8_t* bytes, int& idx)
    {
        uint8_t r = 255 - bytes[idx++];
        uint8_t g = 255 - bytes[idx++];
        uint8_t b = 255 - bytes[idx++];
        uint8_t a = 255 - bytes[idx++];
        
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }
        
    std::string ReadNullTerminatedStringFromBytes(const uint8_t* bytes, int& idx)
    {
        std::string result = "";
    
        uint8_t readbyte = bytes[idx];
        idx++;
        while(readbyte != 0)
        {
            result.push_back((char)readbyte);
            readbyte = bytes[idx];
            idx++;    
        }
        
        return result;
    }    

    void WriteBoolToBytes(bool value, uint8_t* bytes, int& idx)
    {
        bytes[idx] = (uint8_t)value;
        idx++;
    }

    void WriteUInt8ToBytes(uint8_t value, uint8_t* bytes, int& idx)
    {
        bytes[idx] = value;
        idx++;
    }

    void WriteUInt16ToBytes(uint16_t value, uint8_t* bytes, int& idx)
    {
        *(uint16_t*)(&bytes[idx]) = value;
        idx += sizeof(uint16_t);
    }

    void WriteUInt32ToBytes(uint32_t value, uint8_t* bytes, int& idx)
    {
        *(uint32_t*)(&bytes[idx]) = value;
        idx += sizeof(uint32_t);
    }
    
    void WriteFloatToBytes(float value, uint8_t* bytes, int& idx)
    {
        *(float*)(&bytes[idx]) = value;
        idx += sizeof(float);
    }
    
    void WriteUUIDToBytes(const RexUUID &value, uint8_t* bytes, int& idx)
    {
        *(RexUUID*)(&bytes[idx]) = value;
        idx += sizeof(RexUUID);
    }    
    
    void WriteNullTerminatedStringToBytes(const std::string& value, uint8_t* bytes, int& idx)
    {
        const char* c_str = value.c_str();
        
        memcpy(&bytes[idx], c_str, value.length() + 1);
        idx += value.length() + 1;
    }
    
    NameValueMap ParseNameValueMap(const std::string& namevalue)
    {
        // NameValue contains: "FirstName STRING RW SV <firstName>\nLastName STRING RW SV <lastName>"
        // When using rex auth <firstName> contains both first and last name and <lastName> contains the auth server address

        // Split into lines
        NameValueMap map;
        StringVector lines = SplitString(namevalue, '\n');
        for (unsigned i = 0; i < lines.size(); ++i)
        {
            StringVector line = SplitString(lines[i], ' ');
            if (line.size() > 4)
            {
                // First element is the name
                const std::string& name = line[0];
                std::string value;
                // Skip over the STRING RW SV etc. (3 values)
                // Concatenate the rest of the values
                for (unsigned j = 4; j < line.size(); ++j)
                {
                    value += line[j];
                    if (j != line.size()-1)
                        value += " ";
                }
                map[name] = value;
            }
        }

        // Parse auth server address out from the NameValue if it exists
        QString fullname = QString(map["FirstName"].c_str()) + " " + QString(map["LastName"].c_str());
        if (fullname.contains('@'))
        {
            QStringList names;
            if (fullname.contains('@'))
            {
                names = fullname.split(" ");
                foreach(QString name, names)
                {
                    if (name.contains('@'))
                    {
                        map["RexAuth"] = name.toStdString();
                        names.removeOne(name);
                    }
                }
            }
            assert(names[0].size() >= 2);
            map["FirstName"] = names[0].toStdString();
            map["LastName"] = names[1].toStdString();
        }

        return map;
    }
    
    bool ReadTextureEntryBits(uint32_t& bits, int& num_bits, const uint8_t* bytes, int& idx)
    {
        bits = 0;
        num_bits = 0;
        uint8_t byte;
        
        do
        {
            byte = bytes[idx++];
            bits <<= 7;
            bits |= byte & 0x7f;
            num_bits += 7;
        }
        while (byte & 0x80);
        
        return bits != 0;
    }
}
