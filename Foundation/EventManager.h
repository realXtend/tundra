// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EventManager_h
#define incl_Foundation_EventManager_h

#include "ModuleReference.h"
#include "EventDataInterface.h"
#include "CoreThread.h"

#include <qnamespace.h>

class QDomElement;

namespace Foundation
{
    class Framework;
    class ModuleInterface;

    //! Manages event passing between modules.
    /*! See \ref EventSystem for details on how to use.

        \ingroup Foundation_group
     */
    class EventManager
    {
    public:
        struct EventSubscriber;
        typedef boost::shared_ptr<EventSubscriber> EventSubscriberPtr;
        typedef std::vector<EventSubscriberPtr> EventSubscriberVector;
        
        //! Event subscriber tree node. Used internally by EventManager.
        struct EventSubscriber
        {
            EventSubscriber() : priority_(0) {}
            
            ModuleWeakPtr module_;
            std::string module_name_;
            int priority_;
            EventSubscriberVector children_;
        };
        
        //! Delayed event. Used internally by EventManager.
        struct DelayedEvent
        {
            event_category_id_t category_id_;
            event_id_t event_id_;
            EventDataPtr data_;
            f64 delay_;
        };
        
        EventManager(Framework *framework);
        ~EventManager();
        
        //! Registers an event category by name
        /*! If event category already registered, will return the existing ID
            \param name New event category name
            \return Non-zero id to event category
         */
        event_category_id_t RegisterEventCategory(const std::string& name);
        
        //! Queries for an event category ID by name
        /*! \param name Event category name
            \return Non-zero id, or zero if event category not recognized
         */
        event_category_id_t QueryEventCategory(const std::string& name) const;
        
       //! Queries for an event category name by ID
        /*! \param category_id event category ID
            \return event category name, or empty string if event category not recognized
         */
        const std::string& QueryEventCategoryName(event_category_id_t category_id) const;
        
        //! Registers an event
        /*! Currently only for debugging purposes, not necessary to send the event in question
        /*! \param category_id Event category ID, should be registered first
            \param event_id Event ID
            \param name Event name
         */
        void RegisterEvent(event_category_id_t category_id, event_id_t event_id, const std::string& name);
        
        //! Queries an event name by category & event ID
        void QueryEventName(event_category_id_t category_id, event_id_t event_id) const;
         
        //! Sends an event
        /*! \param category_id Event category ID
            \param event_id Event ID
            \param data Pointer to event data structure (event-specific), can be 0 if not needed
            \return true if event was handled by some event handler
         */
        bool SendEvent(event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) const;
        
       //! Sends a delayed event
        /*! Use with judgement. Note that you will not get to know whether event was handled. The event data object
            will be retained until event sent, so it should be allocated with new and wrapped inside a shared pointer.
            Delayed events are also the only safe way to send events from threads other than main thread!
            \param category_id Event category ID
            \param event_id Event ID
            \param data Shared pointer to event data structure (event-specific), can be 0 if not needed
            \param delay Delay in seconds until sending event, 0 to send during next framework update
         */
        void SendDelayedEvent(event_category_id_t category_id, event_id_t event_id, EventDataPtr data, f64 delay = 0.0);

        //! Template version of sending a delayed event. Will perform dynamic_pointer_cast from specified type to EventDataPtr
        /*! \param category_id Event category ID
            \param event_id Event ID
            \param data Shared pointer to event data structure (event-specific), can be 0 if not needed
            \param delay Delay in seconds until sending event, 0 to send during next framework update
         */
        template <class T> void SendDelayedEvent(event_category_id_t category_id, event_id_t event_id, boost::shared_ptr<T> data, f64 delay = 0.0)
        {
            SendDelayedEvent(category_id, event_id, boost::dynamic_pointer_cast<EventDataInterface>(data), delay);
        }

        //! Registers a module to the event subscriber tree
        /*! Do not call while responding to an event!
            \param module Module to register
            \param priority Priority among siblings, higher priority = gets called first
            \param parent Subscriber module to use as a parent, 0 to place into the tree root
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
        
        //! Validates event subscriber tree when modules have been loaded/unloaded. Called by ModuleManager.
        void ValidateEventSubscriberTree();
        
        //! Processes delayed events. Called by the framework.
        /*! \param frametime Time since last frame
         */ 
        void ProcessDelayedEvents(f64 frametime);
        
        //! Loads event subscriber tree from an XML file
        /*! \param filename Path/filename of XML file
         */
        void LoadEventSubscriberTree(const std::string& filename);

        typedef std::map<std::string, event_category_id_t> EventCategoryMap;

        typedef std::map<event_category_id_t, std::map<event_id_t, std::string > > EventMap;

        //! Returns event category map
        const EventCategoryMap &GetEventCategoryMap() const { return event_category_map_; }
        
        //! Returns event map
        const EventMap &GetEventMap() const { return event_map_; }
         
        //! Returns next unused non-zero request tag for asset/resource request events
        /*! By having a global source for the tags there is no risk for collisions between
            different modules/subsystems.
         */
        request_tag_t GetNextRequestTag();
        
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
        bool SendEvent(EventSubscriber* node, event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) const;
        
        //! Populates subscriber tree from xml elements
        /*! \param node Pointer to xml element
            \param parent_name Current parent module name (empty for root)
         */
        void BuildTreeFromNode(QDomElement& elem, const std::string parent_name);

        //! Next event category ID that will be assigned
        event_category_id_t next_category_id_;
        
        //! Next free request tag to be used
        request_tag_t next_request_tag_;
                
        //! Map for assigned event category id's
        EventCategoryMap event_category_map_;

        //! Map for registered events by category
        EventMap event_map_;
        
        //! Event subscriber tree root node
        EventSubscriberPtr event_subscriber_root_;
      
        //! Delayed events
        typedef std::vector<DelayedEvent> DelayedEventVector;
        DelayedEventVector new_delayed_events_;
        DelayedEventVector delayed_events_;
        
        //! Mutex for new delayed events
        Mutex delayed_events_mutex_;
        
        //! Framework
        Framework *framework_;
        
        //! Current thread ID
        Qt::HANDLE main_thread_id_;
    };
}

#endif