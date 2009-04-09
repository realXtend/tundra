// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_OpenSimPrim_h
#define incl_EC_OpenSimPrim_h

#include "ComponentInterface.h"
#include "Foundation.h"
#include "RexUUID.h"
#include "OpenSimProtocolModule.h"

namespace RexLogic
{
    class EC_OpenSimPrim : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_OpenSimPrim);
    public:
        virtual ~EC_OpenSimPrim();

        // !ID related
        uint64_t RegionHandle;
        uint32_t LocalId;
        RexUUID FullId;
        uint32_t ParentId; 
        
        std::string ObjectName;
        std::string Description;
        std::string HoveringText;
        std::string MediaUrl;

        uint8_t Material;
        uint8_t ClickAction;
        uint32_t UpdateFlags;
        
        Vector3 Position;
        Vector3 Scale;
        Quaternion Rotation;

        std::string ServerScriptClass;
        
        RexUUID CollisionMesh;
        
        RexUUID SoundUUID;
        float SoundVolume;
        float SoundRadius;
        
        uint32_t SelectPriority;

        void HandleObjectUpdate(OpenSimProtocol::NetworkEventInboundData* data);
        void HandleRexPrimData(const uint8_t* primdata);

        void PrintDebug();
        
    private:
        EC_OpenSimPrim(Foundation::ModuleInterface* module);
    };
}

#endif
