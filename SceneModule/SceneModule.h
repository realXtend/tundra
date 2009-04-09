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
    //! Scene event id's.
    static const Core::event_id_t EVENT_SCENE_ADDED =       0x01;
    static const Core::event_id_t EVENT_SCENE_DELETED =     0x02;
    static const Core::event_id_t EVENT_SCENE_CLONED =      0x03;
    
    //! Entity event id's.
    static const Core::event_id_t EVENT_ENTITY_ADDED =      0x04;
    static const Core::event_id_t EVENT_ENTITY_UPDATED =    0x05;
    static const Core::event_id_t EVENT_ENTITY_DELETED =    0x06;
    static const Core::event_id_t EVENT_ENTITY_SELECT =     0x07;
    static const Core::event_id_t EVENT_ENTITY_SELECTED =   0x08;
    static const Core::event_id_t EVENT_ENTITY_DESELECT =   0x09;
    
    //! Component event id's.
    static const Core::event_id_t EVENT_COMPONENT_ADDED =   0x0a;
    static const Core::event_id_t EVENT_COMPONENT_DELETED = 0x0b;
    
    //! Event data interface for entity events.
    class SceneEventData: public Foundation::EventDataInterface
    {
    public:
        SceneEventData(const std::string &scene_name) : sceneName(scene_name) {}
        SceneEventData(Core::entity_id_t id) : localID(id) {}
        virtual ~SceneEventData() {}
        
        Core::entity_id_t localID;
        std::string sceneName;
    };
        
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
