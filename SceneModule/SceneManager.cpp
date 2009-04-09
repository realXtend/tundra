// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SceneManager.h"
#include "Generic.h"
#include "SceneModule.h"

namespace Scene
{
    Foundation::ScenePtr SceneManager::CreateScene(const std::string &name)
    {
        assert (module_);
        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(name, module_));
        
        scenes_[name] = scene;
        
        ///\todo Make this work.        
        SceneEventData event_data(name);
        Core::event_category_id_t cat_id = module_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
        module_->GetFramework()->GetEventManager()->SendEvent(cat_id, EVENT_SCENE_ADDED, &event_data);
        
        return scene;
    }

    void SceneManager::DeleteScene(const std::string &name)
    {
        assert (HasScene(name) == true);

        SceneMap::iterator it = scenes_.find(name);
        it->second.reset();
        scenes_.erase(it);
        
        assert (HasScene(name) == false);

        SceneEventData event_data(name);
        Core::event_category_id_t cat_id = module_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
        module_->GetFramework()->GetEventManager()->SendEvent(cat_id, EVENT_SCENE_DELETED, &event_data);
    }

    Foundation::ScenePtr SceneManager::CloneScene(const std::string &name, const std::string &cloneName)
    {
        assert (HasScene(name));
        assert (HasScene(cloneName) == false);
        
        Generic *oldScene = checked_static_cast<Generic*>(scenes_.find(name)->second.get());

        Foundation::ScenePtr scene = Foundation::ScenePtr(new Scene::Generic(*oldScene, cloneName));

        scenes_[cloneName] = scene;

        assert (HasScene(cloneName));
        
        ///\todo
//      Core::event_category_id_t cat_id = module_->GetFramework()->GetEventManager()->QueryEventCategory("Scene");
//      module_->GetFramework()->GetEventManager()->SendEvent(0, EVENT_SCENE_CLONED, NULL);

        return scene;
    }
}

