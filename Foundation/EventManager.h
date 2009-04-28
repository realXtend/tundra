// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EventManager_h
#define incl_Foundation_EventManager_h

#include "ModuleReference.h"

namespace Poco
{
    namespace XML
    {
        class Node;
    }
}

namespace Foundation
{
    class Framework;
    class EventDataInterface;
    class ModuleInterface;

    //! Manages event passing between modules.
    /*! See \ref EventSystem for details on how to use.  
     */
    class EventManager
    {
    public:
        class EventSubscriber;
        typedef boost::shared_ptr<EventSubscriber> EventSubscriberPtr;
        typedef std::vector<EventSubscriberPtr> EventSubscriberVector;
        
        //! Event subscriber tree node. Used internally by EventManager.
        class EventSubscriber
        {
        public:
            EventSubscriber() : priority_(0) {}
            
            ModuleWeakPtr module_;
            std::string module_name_;
            int priority_;
            EventSubscriberVector children_;
        };
        
        EventManager(Framework *framework);
        ~EventManager();
        
        //! Registers an event category by name
        /*! If event category already registered, will return the existing ID
            \param name New event category name
            \return Non-zero id to event category
         */
        Core::event_category_id_t RegisterEventCategory(const std::string& name);
        
        //! Queries for an event category ID by name
        /*! \param name Event category name
            \return Non-zero id, or zero if event category not recognized
         */
        Core::event_category_id_t QueryEventCategory(const std::string& name) const;
        
       //! Queries for an event category name by ID
        /*! \param category_id event category ID
            \return event category name, or empty string if event category not recognized
         */
        const std::string& QueryEventCategoryName(Core::event_category_id_t category_id) const;
        
        //! Registers an event
        /*! Currently only for debugging purposes, not necessary to send the event in question
        /*! \param category_id Event category ID, should be registered first
            \param event_id Event ID
            \param name Event name
         */
        void RegisterEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, const std::string& name);
        
        //! Queries an event name by category & event ID
        void QueryEventName(Core::event_category_id_t category_id, Core::event_id_t event_id) const;
         
        //! Sends an event
        /*! \param category_id Event category ID
            \param event_id Event ID
            \param data Pointer to event data structure (event-specific), can be NULL if not needed
            \return true if event was handled by some event handler
         */
        bool SendEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) const;
        
        //! Registers a module to the event subscriber tree
        /*! Do not call while responding to an event!
            \param module Module to register
            \param priority Priority among siblings, higher priority = gets called first
            \param parent Subscriber module to use as a parent, NULL to place into the tree root
            \return true if successfully subscribed
         */
        bool RegisterEventSubscriber(ModuleWeakPtr module, int priority, ModuleWeakPtr parent);

        //! Unregisters a module from the subscriber tree
        /*! Do not call while responding to an event!
            Note: possible children will be unsubscribed as well
            \param module Module to unregister
            \return true if successfully unsubscribed
         */
        bool UnregisterEventSubscriber(ModuleInterface* module);
        
        //! Checks if module is registered as an event subscriber
        /*! \param module Module to check
            \return true if is registered
         */
        bool HasEventSubscriber(ModuleInterface* module);
        
        //! Validates event subscriber tree when modules have been loaded/unloaded
        void ValidateEventSubscriberTree();
        
        //! Loads event subscriber tree from an XML file
        /*! \param filename Path/filename of XML file
         */
        void LoadEventSubscriberTree(const std::string& filename);

        typedef std::map<std::string, Core::event_category_id_t> EventCategoryMap;

        typedef std::map<Core::event_category_id_t, std::map<Core::event_category_id_t, std::string > > EventMap;

        //! Returns event category map
        const EventCategoryMap &GetEventCategoryMap() const { return event_category_map_; }
        
        //! Returns event map
        const EventMap &GetEventMap() const { return event_map_; }
         
    private:
        //! Validates event subscriber tree when modules have been loaded/unloaded
        void ValidateEventSubscriberTree(EventSubscriber* node);
        
        //! Finds a node with certain module from the tree
        /*! \param node Starting node for search
            \param module Module to look for
            \return Pointer to node, or 0 if not found
         */
        EventSubscriber* FindNodeWithModule(EventSubscriber* node, ModuleInterface* module) const;
        
        //! Finds a node from the tree which has certain module as a child
         /*! \param node Starting node for search
            \param module Module to look for
            \return Pointer to node, or 0 if not found
         */
        EventSubscriber* FindNodeWithChild(EventSubscriber* node, ModuleInterface* module) const;
        
        //! Sends event to a module in the subscriber tree, propagate to children as necessary
        /*! \param node Which tree node to send to
            \param category_id Event category ID
            \param event_id Event ID
            \param data Pointer to event data structure (event-specific)
            \return true if event handled and further nodes should not be processed
         */
        bool SendEvent(EventSubscriber* node, Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) const;
        
        //! Populates subscriber tree from Poco xml document nodes
        /*! \param node Pointer to Poco xml document node
            \param parent_name Current parent module name (empty for root)
         */
        void BuildTreeFromNode(Poco::XML::Node* node, const std::string parent_name);

        //! Next event category ID that will be assigned
        Core::event_category_id_t next_category_id_;
        
        //! Map for assigned event category id's
        EventCategoryMap event_category_map_;

        //! Map for registered events by category
        EventMap event_map_;
        
        //! Event subscriber tree root node
        EventSubscriberPtr event_subscriber_root_;
      
        //! Framework
        Framework *framework_;
    };
}

#endif