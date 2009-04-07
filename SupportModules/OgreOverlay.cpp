// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include "OgreOverlay.h"
#include "EC_OgreConsoleOverlay.h"

namespace Console
{
    OgreOverlay::OgreOverlay(Foundation::ModuleInterface *module) : Console::ConsoleServiceInterface(), module_(module)
    {
        Foundation::Framework *framework = module_->GetFramework();

        if ( framework->GetServiceManager()->IsRegistered(Foundation::Service::ST_SceneManager) &&
             framework->GetComponentManager()->CanCreate("EC_OgreConsoleOverlay") )
        {
            Foundation::SceneManagerServiceInterface *scene_manager =
                framework->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);

            if (scene_manager->HasScene("Console"))
                throw Core::Exception("Scene for console already exists.");

            Foundation::ScenePtr scene = scene_manager->CreateScene("Console");
            Foundation::EntityPtr entity = scene->CreateEntity(scene->GetNextFreeId());

            console_overlay_ = framework->GetComponentManager()->CreateComponent("EC_OgreConsoleOverlay");
            entity->AddEntityComponent(console_overlay_);
        }
    }

    OgreOverlay::~OgreOverlay()
    {
        Foundation::Framework *framework = module_->GetFramework();

        if ( framework->GetServiceManager()->IsRegistered(Foundation::Service::ST_SceneManager))
        {
            Foundation::SceneManagerServiceInterface *scene_manager =
                framework->GetService<Foundation::SceneManagerServiceInterface>(Foundation::Service::ST_SceneManager);

            if (scene_manager->HasScene("Console"))
            {
                scene_manager->DeleteScene("Console");
            }
        }
    }

    // virtual
    void OgreOverlay::Print(const std::string &text)
    {
        if (console_overlay_)
            checked_static_cast<OgreRenderer::EC_OgreConsoleOverlay*>(console_overlay_.get())->Print(text);
    }
}
