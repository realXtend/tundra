// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneEvents_h
#define incl_SceneEvents_h

#include "EventDataInterface.h"
#include "Vector3D.h"
#include "ForwardDefines.h"

namespace Scene
{
    class Entity;
    //! Events related to scene
    /*!
        \ingroup Scene_group
    */
    namespace Events
    {
        // Scene-related event id's:

        /// Sent as a notification whenever a new scene was added to the system.
        ///\todo Not yet used. \todo Event data object not defined.
        static const event_id_t EVENT_SCENE_ADDED =       0x01;

        /// Sent as a notification whenever a scene was deleted from the system.
        ///\todo Not yet used. \todo Event data object not defined.
        static const event_id_t EVENT_SCENE_DELETED =     0x02;

        /// Sent as a notification whenever a clone of an existing scene was made.
        ///\todo Not yet used. \todo Event data object not defined.
        ///\todo This might never be supported if not useful/necessary.. remove?
        static const event_id_t EVENT_SCENE_CLONED =      0x03;

        // Entity-related event id's:

        /// Sent as a notification whenever a new entity was added to a scene.
        ///\todo Not yet used.
        static const event_id_t EVENT_ENTITY_ADDED =      0x04;

        /// Sent as a notification whenever an attribute in a component of an entity was modified.
        ///\todo Not yet used.
        ///\todo Event data object not defined.
        ///\todo Might be too general? Break down into component-specific events?
        static const event_id_t EVENT_ENTITY_UPDATED =    0x05;

        /// Sent by the SceneManager::DeleteEntity as a notification whenever an entity was deleted from a scene.
        static const event_id_t EVENT_ENTITY_DELETED =    0x06;

        /// An action event that can be sent by anyone to cause the RexLogic module change the currently selected
        /// objects and fetch their properties from the server. \todo Not yet used.
        static const event_id_t EVENT_ENTITY_SELECT =     0x07;

        /// Sent by the RexLogic primitive handler as a notification whenever the current selection was changed 
        /// and the object properties have been fetched from the server.
        static const event_id_t EVENT_ENTITY_SELECTED =   0x08;

        /// An action event that can be sent by anyone to cause the current selection to be cleared. \todo Not yet used.
        static const event_id_t EVENT_ENTITY_DESELECT =   0x09;

        /// An action event that can be sent by anyone to cause an entity be grabbed (drag-selected).
        ///\todo Distinguish which clicks need to cause a select and which need to cause a grab.
        static const event_id_t EVENT_ENTITY_GRAB =       0x0a;

        //Sent when object clicked. Same as EVENT_ENTITY_GRAB but sends a entity pointer, not just id
        static const event_id_t EVENT_ENTITY_CLICKED =    0x0b;

        // Component-related event id's:

    /** \todo For creating events that can be used to alter the scene, events like this might be useful as well:
        /// Indicates that a new component is added to an entity.
        static const event_id_t EVENT_COMPONENT_ADDED =   0x0b;
        /// Indicates that a component is deleted from an entity.
        static const event_id_t EVENT_COMPONENT_DELETED = 0x0c;
        /// Indicateds that component's properties are updated.
        static const event_id_t EVENT_COMPONENT_UPDATED=  0x0d;
*/
        // Other events:

        /// Sent to register a scene entity as a controllable entity (has the Controllable component). Also sets
        /// that entity as the currently controlled entity. 
        static const event_id_t EVENT_CONTROLLABLE_ENTITY = 0x0e;

        /// An internal event telling that an entity's visible geometry or materials has been regenerated or modified.
        static const event_id_t EVENT_ENTITY_VISUALS_MODIFIED = 0x0f;

        /// An action event that can be sent by anyone to create a new entity.
        static const event_id_t EVENT_ENTITY_CREATE = 0x10;

        /// An event that will let mediaurl handling to know for what entity a mediaurl was set
        static const event_id_t EVENT_ENTITY_MEDIAURL_SET = 0x11; //REMOTE == from server

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
            EntityEventData(entity_id_t id) : localID(id) {}
            virtual ~EntityEventData() {}
            
            entity_id_t localID;
            
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
            EntityEventData(entity_id_t id) : localID(id) {}
            virtual ~EntityEventData() {}
            
            entity_id_t localID;
            
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
            explicit SceneEventData(const std::string &scene_name);
            explicit SceneEventData(entity_id_t id);
            virtual ~SceneEventData();
            
            entity_id_t localID;
            std::string sceneName;
            std::vector<Scene::EntityPtr> entity_ptr_list;
        };

        class EntityEventData : public Foundation::EventDataInterface
        {
        public:
            Scene::EntityPtr entity;
        };

        class RaycastEventData : public SceneEventData
        {
        public:
            explicit RaycastEventData(entity_id_t id);
            virtual ~RaycastEventData();
            
             //! World coordinates of hit position
            Vector3df pos;
            //! Submesh index in entity, starting from 0
            uint submesh;
            //! U coord in entity. 0 if no texture mapping
            Real u;
            //! V coord in entity. 0 if no texture mapping
            Real v;
        };

        class EntityClickedData : public SceneEventData
        {
        public:
            explicit EntityClickedData(Scene::Entity *ent);
            virtual ~EntityClickedData();
            
            //Entity that was clicked
            Scene::Entity *entity;
        };

        class CreateEntityEventData : public Foundation::EventDataInterface
        {
        public:
            CreateEntityEventData(Vector3df pos);
            virtual ~CreateEntityEventData();
            Vector3df position;
        };

        class TerrainTexturesEventData : public Foundation::EventDataInterface
        {
        public:
            /// should be same as RexTypes::RexAssetID. std::string is used because we dont want more dependences.
            std::string terrain[4];
        };

        class WaterEventData : public Foundation::EventDataInterface
        {
        public:
            WaterEventData();
            explicit WaterEventData(float h);
            virtual ~WaterEventData();
            /// Water height 
            float height;
        };

        void RegisterSceneEvents(const Foundation::EventManagerPtr &event_manager);
    }
}

#endif
