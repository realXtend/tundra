// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexObject_h
#define incl_RexObject_h

#include "RexTypes.h"

namespace RexLogic
{
    class RexLogicModule;

    class RexObject
    {
     public:
        RexObject(RexLogicModule *rexlogicmodule);
        virtual ~RexObject();
             
    protected:     
        RexLogicModule *rexlogicmodule_;
        
        //! Creates an OBB for debug visualization of the extents of the given scene object.
        void DebugCreateOgreBoundingBox(const Foundation::ComponentInterfacePtr ogrePlaceable, const std::string &materialName);
        
        //! Helper functions for getting values from terseupdate packets
        Core::Quaternion GetProcessedQuaternion(const uint8_t* bytes);
        Core::Vector3df GetProcessedScaledVectorFromUint16(const uint8_t* bytes, float scale);
        Core::Vector3df GetProcessedVectorFromUint16(const uint8_t* bytes);
        Core::Vector3df GetProcessedVector(const uint8_t* bytes);                
    };
}

#endif