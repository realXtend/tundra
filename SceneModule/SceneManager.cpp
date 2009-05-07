// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneManager.h"
#include "Generic.h"
#include "SceneModule.h"
#include "SceneEvents.h"

namespace Scene
{
    Foundation::ScenePtr SceneManager::CreateScene(const std::string &name)
    {
        assert (module_);
        
        if (scenes_.find(name) != scenes_.end())
        {
            //! \todo Is warning to log enough, or should we assert? -cm
            SceneModule::LogWarning("Overriding scene " + name + " with a new empty scene.");
        }

        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(name, module_));

        scenes_[name] = scene;
        
        ///\todo Make this work.
        Events::SceneEventData event_data(name);
        Core::event_category_id_t cat_id = module_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
        module_->GetFramework()->GetEventManager()->SendEvent(cat_id, Events::EVENT_SCENE_ADDED, &event_data);
        
        return scene;
    }

    void SceneManager::DeleteScene(const std::string &name)
    {
        assert (HasScene(name) == true);

        SceneMap::iterator it = scenes_.find(name);
        it->second.reset();
        scenes_.erase(it);
        
        assert (HasScene(name) == false);

        Events::SceneEventData event_data(name);
        Core::event_category_id_t cat_id = module_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
        module_->GetFramework()->GetEventManager()->SendEvent(cat_id, Events::EVENT_SCENE_DELETED, &event_data);
    }

    Foundation::ScenePtr SceneManager::CloneScene(const std::string &name, const std::string &cloneName)
    {
        assert (HasScene(name));
        assert (HasScene(cloneName) == false);
        
        Generic *oldScene = checked_static_cast<Generic*>(scenes_.find(name)->second.get());

        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(*oldScene, cloneName));

        scenes_[cloneName] = scene;

        assert (HasScene(cloneName));
        
        ///\todo Make this work.
//      Core::event_category_id_t cat_id = module_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
//      module_->GetFramework()->GetEventManager()->SendEvent(0, EVENT_SCENE_CLONED, NULL);

        return scene;
    }

    Foundation::ScenePtr SceneManager::GetScene(const std::string &name) const
    {
        SceneMap::const_iterator it = scenes_.find(name);
        if (it != scenes_.end())
            return it->second;

        SceneModule::LogDebug("SceneManager::GetScene: Scene " + name + " not found.");
        
        return Foundation::ScenePtr();
    }
}

