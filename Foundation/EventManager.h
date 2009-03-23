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

    //! Manages event passing between modules.
    /*! To use, you should first register an event category during your module's Initialize() function.
        To register, pass your event category name to RegisterEventCategory. Name can be similar to your module's name,
        for example the Ogre rendering module uses the event category name "Renderer". You get back a category
        ID, which you should store.
        
        Then, during your module's PostInitialize(), you can assume that all modules have registered their event 
        categories, and you can start querying for other categories you care about with QueryEventCategory().
        
        To send an event, use the function SendEvent(). You need the category ID and an event ID, which you can come up
        with yourself, for the event ID only needs to be unique within your event category. Additionally SendEvent()
        takes a pointer to an event-specific event data object, which has to be a subclass of EventDataInterface. 
        If no data is required, this pointer can be null.
        
        In the ModuleInterface, there is a corresponding HandleEvent() function, which gets the category ID, event ID 
        and the data pointer as parameters. Return true from HandleEvent() if you handled the event and do not wish 
        for it to propagate further in the event subscriber hierarchy, false otherwise.
        
        To actually receive events, modules need to be registered as event subscribers. They are organized into a tree-
        structure which is read from the xml data file /data/event_tree.xml. In the data file, modules are referred to
        by their names. Additionally, modules need an integer priority number (higher number = higher priority), which
        decides the order in which HandleEvent() is called for siblings in the tree.
        
        An example subscriber tree data file, does not necessarily make sense:
        
        \verbatim
        <subscribers>
            <subscriber module="Renderer" priority="75" />
            <subscriber module="World Logic" priority="100" />
                <subscriber module="Gui" priority="100" />
            </subscriber>
        </subscribers>
        \endverbatim
        
        Here, the tree root has two children: Renderer and World Logic. The Renderer will have lower priority than World 
        Logic, so World Logic (and its child Gui) get the first opportunity to handle events.
        
        Modules can also be manually registered/unregistered into the subscriber tree by calling RegisterEventSubscriber()
        and UnregisterEventSubscriber(). Note that during handling of an event (ie. when HandleEvent() for any module is 
        being executed) the subscriber tree should not be attempted to be modified.
        
        Additionally (currently only for debugging purposes, not mandatory to send events) event id's can be registered. 
     */
    
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
        /*! \param category_id event category id
            \return event category name, or empty string if event category not recognized
         */
        const std::string& QueryEventCategoryName(Core::event_category_id_t category_id) const;
        
        //! registers an event
        /*! currently only for debugging purposes, not necessary to send the event in question
        /*! \param category_id event category id, should be registered first
            \param event_id event id
            \param name event name
         */
        void RegisterEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, const std::string& name);
        
        //! Queries an event name by category & event id
        void QueryEventName(Core::event_category_id_t category_id, Core::event_id_t event_id) const;
         
        //! sends an event
        /*! \param category_id event category id
            \param event_id event id
            \param data pointer to event data structure (event-specific)
         */
        void SendEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) const;
        
        //! registers a module to the event subscriber tree
        /*! do not call while responding to an event!
            \param module module to register
            \param priority priority among siblings, higher priority = gets called first
            \param parent subscriber module to use as a parent, NULL to place into the tree root
            \return true if successfully subscribed
         */
        bool RegisterEventSubscriber(ModuleInterface* module, int priority, ModuleInterface* parent);

        //! unregisters a module from the subscriber tree
        /*! do not call while responding to an event!
            note: possible children will be unsubscribed as well
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

        typedef std::map<std::string, Core::event_category_id_t> EventCategoryMap;

        typedef std::map<Core::event_category_id_t, std::map<Core::event_category_id_t, std::string > > EventMap;

        //! returns event category map
        const EventCategoryMap &GetEventCategoryMap() const { return event_category_map_; }
        
        //! returns event map
        const EventMap &GetEventMap() const { return event_map_; }
         
    private:
        
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

        //! map for registered events by category
        EventMap event_map_;
        
        //! event subscriber tree root node
        EventSubscriberPtr event_subscriber_root_;
        
        Framework *framework_;
    };
}

#endif