// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MumbleVoipModule.h"
#include "MemoryLeakCheck.h"
#include "LinkPlugin.h"
#include "RexLogicModule.h"
#include "ModuleManager.h"
#include "Avatar/Avatar.h"
#include "EC_OgrePlaceable.h"
#include "SceneManager.h"


namespace MumbleVoip
{
	std::string MumbleVoipModule::module_name_ = "MumbleVoipModule";

	MumbleVoipModule::MumbleVoipModule()
        : ModuleInterfaceImpl(module_name_), link_plugin_(new LinkPlugin())
    {
    }

    MumbleVoipModule::~MumbleVoipModule()
    {
        SAFE_DELETE(link_plugin_);
    }

    void MumbleVoipModule::Load()
    {
    }

    void MumbleVoipModule::Unload()
    {
    }

    void MumbleVoipModule::Initialize() 
    {
		InitializeLinkPlugin();
    }

    void MumbleVoipModule::PostInitialize()
    {
        // Testing...
        link_plugin_->SetAvatarName("Test User");
        link_plugin_->SetAvatarIdentity("test_user_1231412389483478");
        link_plugin_->SetGroupId("naali-test-users");
        link_plugin_->SetDescription("Naali viewer");
    }

    void MumbleVoipModule::Uninitialize()
    {
    }

    void MumbleVoipModule::Update(f64 frametime)
    {
        RexLogic::RexLogicModule *rex_logic_module = dynamic_cast<RexLogic::RexLogicModule *>(framework_->GetModuleManager()->GetModule(Foundation::Module::MT_WorldLogic).lock().get());
        if (!rex_logic_module)
            return;

        RexLogic::AvatarPtr avatar = rex_logic_module->GetAvatarHandler();
        if (!avatar)
            return;

        Scene::EntityPtr entity = avatar->GetUserAvatar();
        if (!entity)
            return;

        const Foundation::ComponentInterfacePtr &placeable_component = entity->GetComponent("EC_OgrePlaceable");
        if (placeable_component)
        {
            OgreRenderer::EC_OgrePlaceable *ogre_placeable = checked_static_cast<OgreRenderer::EC_OgrePlaceable *>(placeable_component.get());
            Quaternion q = ogre_placeable->GetOrientation();
            Vector3df position_vector = ogre_placeable->GetPosition(); 
            Vector3df top_vector(0,0,1);
            Vector3df front_vector(q.x,q.y,q.y);
            link_plugin_->SetAvatarPosition(position_vector, front_vector, top_vector);
            link_plugin_->SetCameraPosition(position_vector, front_vector, top_vector); //todo: use real values from camera
        }

        link_plugin_->SendData();
    }

	bool MumbleVoipModule::HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data)
	{
		return false;
	}

	void MumbleVoipModule::InitializeLinkPlugin()
	{
		//! todo: IMPLEMENT
	}
} // end of namespace: MumbleVoip

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace MumbleVoip;

POCO_BEGIN_MANIFEST(Foundation::ModuleInterface)
POCO_EXPORT_CLASS(MumbleVoipModule)
POCO_END_MANIFEST
