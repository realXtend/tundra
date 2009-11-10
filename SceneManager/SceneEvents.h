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
        // Scene-related event id's:

        /// Sent as a notification whenever a new scene was added to the system.
        ///\todo Not yet used. \todo Event data object not defined.
        static const Core::event_id_t EVENT_SCENE_ADDED =       0x01;

        /// Sent as a notification whenever a scene was deleted from the system.
        ///\todo Not yet used. \todo Event data object not defined.
        static const Core::event_id_t EVENT_SCENE_DELETED =     0x02;

        /// Sent as a notification whenever a clone of an existing scene was made.
        ///\todo Not yet used. \todo Event data object not defined.
        ///\todo This might never be supported if not useful/necessary.. remove?
        static const Core::event_id_t EVENT_SCENE_CLONED =      0x03;

        // Entity-related event id's:

        /// Sent as a notification whenever a new entity was added to a scene.
        ///\todo Not yet used.
        static const Core::event_id_t EVENT_ENTITY_ADDED =      0x04;

        /// Sent as a notification whenever an attribute in a component of an entity was modified.
        ///\todo Not yet used.
        ///\todo Event data object not defined.
        ///\todo Might be too general? Break down into component-specific events?
        static const Core::event_id_t EVENT_ENTITY_UPDATED =    0x05;

        /// Sent by the SceneManager::DeleteEntity as a notification whenever an entity was deleted from a scene.
        static const Core::event_id_t EVENT_ENTITY_DELETED =    0x06;

        /// An action event that can be sent by anyone to cause the RexLogic module change the currently selected
        /// objects and fetch their properties from the server. \todo Not yet used.
        static const Core::event_id_t EVENT_ENTITY_SELECT =     0x07;

        /// Sent by the RexLogic primitive handler as a notification whenever the current selection was changed 
        /// and the object properties have been fetched from the server.
        static const Core::event_id_t EVENT_ENTITY_SELECTED =   0x08;

        /// An action event that can be sent by anyone to cause the current selection to be cleared. \todo Not yet used.
        static const Core::event_id_t EVENT_ENTITY_DESELECT =   0x09;

        /// An action event that can be sent by anyone to cause an entity be grabbed (drag-selected).
        ///\todo Distinguish which clicks need to cause a select and which need to cause a grab.
        static const Core::event_id_t EVENT_ENTITY_GRAB =       0x0a;

        // Component-related event id's:

    /** \todo For creating events that can be used to alter the scene, events like this might be useful as well:
        /// Indicates that a new component is added to an entity.
        static const Core::event_id_t EVENT_COMPONENT_ADDED =   0x0b;
        /// Indicates that a component is deleted from an entity.
        static const Core::event_id_t EVENT_COMPONENT_DELETED = 0x0c;
        /// Indicateds that component's properties are updated.
        static const Core::event_id_t EVENT_COMPONENT_UPDATED=  0x0d;
*/
        // Other events:

        /// Sent to register a scene entity as a controllable entity (has the Controllable component). Also sets
        /// that entity as the currently controlled entity. 
        static const Core::event_id_t EVENT_CONTROLLABLE_ENTITY = 0x0e;

        /// An internal event telling that an entity's visible geometry or materials has been regenerated or modified.
        static const Core::event_id_t EVENT_ENTITY_VISUALS_MODIFIED = 0x0f;

        /// An action event that can be sent by anyone to create a new entity.
        static const Core::event_id_t EVENT_ENTITY_CREATE = 0x10;

        /// Event data interface for Scene object related events.
        /*class SceneEventData: public Foundation::EventDataInterface
        {
        public:
            SceneEventData(const std::string &scene_name) : sceneName(scene_name) {}
            virtual ~SceneEventData() {}
            
            //! Name of the scene.
            std::string sceneName;
            
            //! List of scene pointers.
            std::vector<Scene::ScenePtr> scene_ptr_list;
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
            std::vector<Scene::EntityPtr> entity_ptr_list;
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
            Scene::EntityPtr ownerEntityPtr;
            
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
            std::vector<Scene::EntityPtr> entity_ptr_list;
        };

        class EntityEventData : public Foundation::EventDataInterface
        {
        public:
            Scene::EntityPtr entity;
        };

        class CreateEntityEventData : public Foundation::EventDataInterface
        {
        public:
            CreateEntityEventData(Core::Vector3df pos) : position(pos){}
            virtual ~CreateEntityEventData() {}
            Core::Vector3df position;
        };

        namespace
        {
            void RegisterSceneEvents(const Foundation::EventManagerPtr &event_manager)
            {
                Core::event_category_id_t scene_event_category = event_manager->RegisterEventCategory("Scene");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_SCENE_ADDED, "Scene Added");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_SCENE_DELETED, "Scene Deleted");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_SCENE_CLONED, "Scene Cloned"); ///\todo
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_ADDED, "Entity Added");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_UPDATED, "Entity Updated");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_DELETED, "Entity Deleted");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_SELECT, "Entity Select");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_SELECTED, "Entity Selected");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_DESELECT, "Entity Deselect");

                event_manager->RegisterEvent(scene_event_category, Events::EVENT_CONTROLLABLE_ENTITY, "Controllable Entity Created");
                event_manager->RegisterEvent(scene_event_category, Events::EVENT_ENTITY_VISUALS_MODIFIED, "Entity Visual Appearance Modified");
            }
        }
    }
}

#endif
