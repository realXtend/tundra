#include "StableHeaders.h"
#include "EntityComponent/ComponentResourceHandler.h"
#include "ResourceInterface.h"
#include "SceneManager.h"
#include "OgreMeshResource.h"

#include "EntityComponent/EC_Mesh.h"

namespace RexLogic
{
ComponentResourceHandler::ComponentResourceHandler(Foundation::ModuleInterface *module):
framework_(module->GetFramework())
{
    
}

void ComponentResourceHandler::HandleResourceEvent(event_id_t event_id, Foundation::EventDataInterface* data)
{
    if (event_id == Resource::Events::RESOURCE_READY)
    {
        /*Resource::Events::ResourceReady* event_data = checked_static_cast<Resource::Events::ResourceReady*>(data);
        OgreRenderer::OgreMeshResource* res = checked_static_cast<OgreRenderer::OgreMeshResource*>(event_data->resource_.get());*/

        Scene::SceneManager *scene = framework_->GetDefaultWorldScene().get();

        // Crashes to null scene ptr when disconnecting while resources are still loading
        if (!scene)
            return;

        Scene::EntityList entities = scene->GetEntitiesWithComponent("EC_Mesh");
        Scene::EntityList::iterator iter = entities.begin();
        for(;iter != entities.end(); iter++)
        {
            //std::vector<EC_Mesh*> components = iter->get()->GetComponents<EC_Mesh>();
            boost::shared_ptr<EC_Mesh> mesh = (*iter)->GetComponent<EC_Mesh>();
            //mesh->HandleResourceEvent(event_id, data);
            mesh->HandleResourceEvent(event_id, data);
        }
    }
}
}