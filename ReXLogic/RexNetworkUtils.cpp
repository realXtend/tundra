// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
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
    void DebugCreateOgreBoundingBox(Foundation::ModuleInterface *module, Foundation::ComponentInterfacePtr ogrePlaceable, const std::string &materialName)
    {
        OgreRenderer::EC_OgrePlaceable &component = dynamic_cast<OgreRenderer::EC_OgrePlaceable&>(*ogrePlaceable.get());
        boost::shared_ptr<OgreRenderer::Renderer> renderer = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer).lock();
        Ogre::SceneManager *sceneMgr = renderer->GetSceneManager();

        ///\todo Quick W.I.P Ogre object naming, refactor. -jj.
        static int c = 0;
        std::stringstream ss;
        ss << "manual " << c++;
        Ogre::ManualObject *manual = sceneMgr->createManualObject(ss.str());
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

        manual->position(v[0]);
        manual->position(v[1]);
        manual->position(v[0]);
        manual->position(v[2]);
        manual->position(v[0]);
        manual->position(v[4]);

        manual->position(v[1]);
        manual->position(v[3]);
        manual->position(v[1]);
        manual->position(v[5]);

        manual->position(v[2]);
        manual->position(v[6]);
        manual->position(v[2]);
        manual->position(v[3]);

        manual->position(v[3]);
        manual->position(v[7]);

        manual->position(v[4]);
        manual->position(v[5]);
        manual->position(v[4]);
        manual->position(v[6]);

        manual->position(v[5]);
        manual->position(v[7]);

        manual->position(v[6]);
        manual->position(v[7]);

        manual->end();
        manual->setDebugDisplayEnabled(true);
       
        Ogre::SceneNode *node = component.GetSceneNode();
        node->attachObject(manual);
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
}