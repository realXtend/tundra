// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexNetworkUtils.h"
#include "RexLogicModule.h"
#include "QuatUtils.h"
#include "ConversionUtils.h"

// Ogre renderer -specific.
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMaterialManager.h>
#include "../OgreRenderingModule/EC_OgrePlaceable.h"
#include "../OgreRenderingModule/Renderer.h"

namespace RexLogic
{    
    /// Creates a bounding box (consisting of lines) into the Ogre scene hierarchy. This function will be removed or refactored later on, once proper material system is present. -jj.
    Ogre::ManualObject *DebugCreateOgreBoundingBox(
        Foundation::ModuleInterface *module,
        Foundation::ComponentInterfacePtr ogrePlaceable,
        const std::string &materialName,
        Core::Vector3df scale)
    {
        OgreRenderer::EC_OgrePlaceable &component = dynamic_cast<OgreRenderer::EC_OgrePlaceable&>(*ogrePlaceable.get());
        boost::shared_ptr<OgreRenderer::Renderer> renderer = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>
            (Foundation::Service::ST_Renderer).lock();
        if (!renderer)
            return 0;

        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

        Ogre::ManualObject *manual = sceneMgr->createManualObject(renderer->GetUniqueObjectName());
        manual->begin(materialName, Ogre::RenderOperation::OT_LINE_LIST);

        const Ogre::Vector3 v[8] = 
        {
            Ogre::Vector3(-0.5,-0.5,-0.5), // 0 ---
            Ogre::Vector3(-0.5,-0.5, 0.5), // 1 --+
            Ogre::Vector3(-0.5, 0.5,-0.5), // 2 -+-
            Ogre::Vector3(-0.5, 0.5, 0.5), // 3 -++
            Ogre::Vector3( 0.5,-0.5,-0.5), // 4 +--
            Ogre::Vector3( 0.5,-0.5, 0.5), // 5 +-+
            Ogre::Vector3( 0.5, 0.5,-0.5), // 6 ++-
            Ogre::Vector3( 0.5, 0.5, 0.5), // 7 +++
        };
        
        Ogre::Vector3 sv[8];
        for (int i = 0; i < 8; ++i)
        {
            sv[i].x = v[i].x * scale.x;
            sv[i].y = v[i].y * scale.y;
            sv[i].z = v[i].z * scale.z;
        }

        manual->position(sv[0]);
        manual->position(sv[1]);
        manual->position(sv[0]);
        manual->position(sv[2]);
        manual->position(sv[0]);
        manual->position(sv[4]);

        manual->position(sv[1]);
        manual->position(sv[3]);
        manual->position(sv[1]);
        manual->position(sv[5]);

        manual->position(sv[2]);
        manual->position(sv[6]);
        manual->position(sv[2]);
        manual->position(sv[3]);

        manual->position(sv[3]);
        manual->position(sv[7]);

        manual->position(sv[4]);
        manual->position(sv[5]);
        manual->position(sv[4]);
        manual->position(sv[6]);

        manual->position(sv[5]);
        manual->position(sv[7]);

        manual->position(sv[6]);
        manual->position(sv[7]);

        manual->end();
        manual->setDebugDisplayEnabled(true);
       
        Ogre::SceneNode *node = component.GetSceneNode();
        node->attachObject(manual);
        
        return manual;
    }

    bool ParseBool(const std::string &value)
    {
        std::string testedvalue = value;
        boost::algorithm::to_lower(testedvalue);
        return (boost::algorithm::starts_with(testedvalue,"true") || boost::algorithm::starts_with(testedvalue,"1")); 
    }
    
    Core::Quaternion GetProcessedQuaternion(const uint8_t* bytes)
    {    
        uint16_t *rot = reinterpret_cast<uint16_t*>((uint16_t*)&bytes[0]);
        Core::Quaternion rotation = Core::UnpackQuaternionFromU16_4(rot);
        rotation.normalize();

        return Core::OpenSimToOgreQuaternion(rotation);
    }

    Core::Vector3df GetProcessedScaledVectorFromUint16(const uint8_t* bytes, float scale)
    {
        uint16_t *vec = reinterpret_cast<uint16_t*>((uint16_t*)&bytes[0]);
        
        Core::Vector3df resultvector;
        resultvector.x = scale * ((vec[0] / 32768.0f) - 1.0f);
        resultvector.y = scale * ((vec[1] / 32768.0f) - 1.0f);
        resultvector.z = scale * ((vec[2] / 32768.0f) - 1.0f);

        return Core::OpenSimToOgreCoordinateAxes(resultvector);        
    }

    Core::Vector3df GetProcessedVectorFromUint16(const uint8_t* bytes)
    {
        uint16_t *vec = reinterpret_cast<uint16_t*>((uint16_t*)&bytes[0]);

        return Core::OpenSimToOgreCoordinateAxes(Core::Vector3df(vec[0],vec[1],vec[2]));    
    }
    
    Core::Vector3df GetProcessedVector(const uint8_t* bytes)
    {
        Core::Vector3df resultvector = *reinterpret_cast<Core::Vector3df*>((Core::Vector3df*)&bytes[0]);

        return Core::OpenSimToOgreCoordinateAxes(resultvector);  
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

    uint32_t ReadUInt32FromBytes(const uint8_t* bytes, int& idx)
    {
        uint32_t result = *(uint32_t*)(&bytes[idx]);
        idx += sizeof(uint32_t);
        
        return result;
    }
    
    float ReadFloatFromBytes(const uint8_t* bytes, int& idx)
    {
        float result = *(float*)(&bytes[idx]);
        idx += sizeof(float);
        
        return result; 
    }     

    RexTypes::RexUUID ReadUUIDFromBytes(const uint8_t* bytes, int& idx)
    {
        RexUUID result = *(RexUUID*)(&bytes[idx]);
        idx += sizeof(RexUUID);
        
        return result; 
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

    void WriteUInt8ToBytes(uint8_t value, uint8_t* bytes, int& idx)
    {
        bytes[idx] = value;
        idx++;
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
}