// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEvents_h
#define incl_SceneEvents_h

#include "EventDataInterface.h"

namespace Scene
{
    //! Events related to scene
    /*!
        \ingroup Scene_group
    */
    namespace Events
    {
        // Scene related event id's.
        /// Indicates that a new scene is added. Triggered by Scene::SceneManager.
        static const Core::event_id_t EVENT_SCENE_ADDED =       0x01;
        /// Indicates that a scene is deleted. Triggered by Scene::SceneManager.
        static const Core::event_id_t EVENT_SCENE_DELETED =     0x02;
        /// Indicates that a scene is cloned. Triggered by Scene::SceneManager. Not used yet.
        static const Core::event_id_t EVENT_SCENE_CLONED =      0x03;

        // Entity related event id's.
        /// Indicates that a new entity is added to a scene. Triggered by Scene::Generic.
        static const Core::event_id_t EVENT_ENTITY_ADDED =      0x04;
        /// Triggered to notify that event's component properties has been modified.
        ///\todo Is this needed?
        static const Core::event_id_t EVENT_ENTITY_UPDATED =    0x05;
        /// Indicates that a entity is deleted from a scene.
        static const Core::event_id_t EVENT_ENTITY_DELETED =    0x06;
        /// Indicates that a entity is selected and it's properties are viewed.
        static const Core::event_id_t EVENT_ENTITY_SELECT =     0x07;
        /// Indicates that the server has responsed to the 'Entity Selected' event.
        static const Core::event_id_t EVENT_ENTITY_SELECTED =   0x08;
        /// Indicates that the entity has been deselected.
        static const Core::event_id_t EVENT_ENTITY_DESELECT =   0x09;
        
        /// Component related event id's.
        /// Indicates that a new component is added to an entity.
        static const Core::event_id_t EVENT_COMPONENT_ADDED =   0x0a;
        /// Indicates that a component is deleted from an entity.
        static const Core::event_id_t EVENT_COMPONENT_DELETED = 0x0b;
        /// Indicateds that component's properties are updated.
        static const Core::event_id_t EVENT_COMPONENT_UPDATED=  0x0c;
        
        /// Event data interface for Scene object related events.
        /*class SceneEventData: public Foundation::EventDataInterface
        {
        public:
            SceneEventData(const std::string &scene_name) : sceneName(scene_name) {}
            virtual ~SceneEventData() {}
            
            //! Name of the scene.
            std::string sceneName;
            
            //! List of scene pointers.
            std::vector<Foundation::ScenePtr> scene_ptr_list;
        };
        
        /// Event data interface for entity events.
        /*class EntityEventData: public Foundation::EventDataInterface
        {
        public:
            EntityEventData(const std::string &scene_name) : sceneName(scene_name) {}
            EntityEventData(Core::entity_id_t id) : localID(id) {}
            virtual ~EntityEventData() {}
            
            Core::entity_id_t localID;
            
            /// Name of scene where this entity belongs to.
            ///\todo Not used yet.
            std::string ownerSceneName;
            
            /// List of entity pointers.
            std::vector<Foundation::EntityPtr> entity_ptr_list;
        };
        
        /// Event data interface for component events.
        /*class EntityEventData: public Foundation::EventDataInterface
        {
        public:
            EntityEventData(const std::string &scene_name) : sceneName(scene_name) {}
            EntityEventData(Core::entity_id_t id) : localID(id) {}
            virtual ~EntityEventData() {}
            
            Core::entity_id_t localID;
            
            /// Name of scene where this entity belongs to.
            std::string ownerEntityName;
            
            /// Pointer to the entity which owns this component.
            Foundation::EntityPtr ownerEntityPtr;
            
            /// List of component pointers.
            std::vector<Foundation::ComponentInterface> entity_ptr_list;
        };*/
        
        /// Soon to be deprecated class event data class.
        class SceneEventData: public Foundation::EventDataInterface
        {
        public:
            SceneEventData(const std::string &scene_name) : sceneName(scene_name) {}
            SceneEventData(Core::entity_id_t id) : localID(id) {}
            virtual ~SceneEventData() {}
            
            Core::entity_id_t localID;
            std::string sceneName;
            std::vector<Foundation::EntityPtr> entity_ptr_list;
        };
    }
}

#endif
