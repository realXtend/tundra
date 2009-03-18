// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EventManager_h
#define incl_Foundation_EventManager_h

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

    //! Manages event category registering, and event passing between modules.
    class EventManager
    {
    public:
        class EventSubscriber;
        typedef boost::shared_ptr<EventSubscriber> EventSubscriberPtr;
        typedef std::vector<EventSubscriberPtr> EventSubscriberVector;
        
        //! Event subscriber tree node
        class EventSubscriber
        {
        public:
            EventSubscriber() : module_(NULL), priority_(0) {}
            
            ModuleInterface* module_;
            int priority_;
            EventSubscriberVector children_;
        };
        
        EventManager(Framework *framework);
        ~EventManager();
        
        //! registers an event category by name
        /*! if event category already registered, will return the existing id
            \param name new event category name
            \return non-zero id to event category
         */
        Core::event_category_id_t RegisterEventCategory(const std::string& name);
        
        //! queries for an event category id by name
        /*! \param name event category name
            \return non-zero id, or zero if event category not recognized
         */
        Core::event_category_id_t QueryEventCategory(const std::string& name) const;
        
       //! queries for an event category name by id
        /*! \param id event category id
            \return event category name, or empty string if event category not recognized
         */
        const std::string& QueryEventCategoryName(Core::event_category_id_t) const;
        
        //! sends an event
        /*! \param category_id event category id
            \param event_id event id
            \param data pointer to event data structure (event-specific)
         */
        void SendEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) const;
        
        //! registers a module to the subscriber tree
        /*! \param module module to register
            \param priority priority among siblings, higher priority = gets called first
            \param parent subscriber module to use as a parent, NULL to place into the tree root
            \return true if successfully subscribed
         */
        bool RegisterEventSubscriber(ModuleInterface* module, int priority, ModuleInterface* parent);

        //! unregisters a module from the subscriber tree
        /*! note: possible children will be unsubscribed as well
            \param module module to unregister
            \return true if successfully unsubscribed
         */
        bool UnregisterEventSubscriber(ModuleInterface* module);
        
        //! checks if module is registered as an event subscriber
        /*! \param module module to check
            \return true if is registered
         */
        bool HasEventSubscriber(ModuleInterface* module);
        
        //! loads event subscriber tree from an XML file
        /*! \param filename path/filename of XML file
         */
        void LoadEventSubscriberTree(const std::string& filename);

    private:
        typedef std::map<std::string, Core::event_category_id_t> EventCategoryMap;
        
        //! find a node with certain module from the tree
        /*! \param node starting node for search
            \param module module to look for
            \return pointer to node, or 0 if not found
         */
        EventSubscriber* FindNodeWithModule(EventSubscriber* node, ModuleInterface* module) const;
        
        //! find a node from the tree which has certain module as a child
         /*! \param node starting node for search
            \param module module to look for
            \return pointer to node, or 0 if not found
         */
        EventSubscriber* FindNodeWithChild(EventSubscriber* node, ModuleInterface* module) const;
        
        //! send event to a module in subscriber tree, propagate to children as necessary
        /*! \param node which tree node to send to
            \param category_id event category id
            \param event_id event id
            \param data pointer to event data structure (event-specific)
            \return true if event handled and further nodes should not be processed
         */
        bool SendEvent(EventSubscriber* node, Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) const;
        
        //! populate subscriber tree from Poco xml document nodes
        /*! \param node pointer to Poco xml document node
            \param parent_name current parent module name (empty for root)
         */
        void BuildTreeFromNode(Poco::XML::Node* node, const std::string parent_name);

        //! next event category id that will be assigned
        Core::event_category_id_t next_category_id_;
        
        //! map for assigned event category id's
        EventCategoryMap event_category_map_;
        
        //! event subscriber tree root node
        EventSubscriberPtr event_subscriber_root_;
        
        Framework *framework_;
    };
}

#endif