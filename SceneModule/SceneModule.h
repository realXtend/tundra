// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneModule_h
#define incl_SceneModule_h

#include "ModuleInterface.h"
#include "SceneManager.h"
#include "SceneModuleApi.h"
#include "EventDataInterface.h"

namespace Foundation
{
    class Framework;
}

namespace Scene
{
    //! interface for modules
    class SCENE_MODULE_API SceneModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        SceneModule();
        virtual ~SceneModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        //! Returns the scene manager
        Foundation::SceneManagerPtr GetSceneManager() const { return scene_manager_; }

        //! Returns the parent framework
        Foundation::Framework *GetFramework() const { assert(framework_); return framework_; }

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Scene;
        
    private:
        Foundation::SceneManagerPtr scene_manager_;
        
        //! Scene event category.
        Core::event_category_id_t scene_event_category_;        
    };
}

#endif
