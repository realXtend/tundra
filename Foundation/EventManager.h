// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EventManager_h
#define incl_Foundation_EventManager_h

#include "ModuleReference.h"
#include "IEventData.h"
#include "CoreThread.h"
#include "IComponent.h"
#include "Framework.h"


#include <QList>
#include <QtAlgorithms>
#include <QMap>
#include <QPair>



namespace Foundation
{
   
    class EventManager
    {
        public:
            EventManager(Framework *framework);
            virtual ~EventManager();

            typedef std::map<std::string, event_category_id_t> EventCategoryMap;
            typedef std::map<event_category_id_t, std::map<event_id_t, std::string > > EventMap;

            /* * 
             *  Registers an event category by name
             *  if event category already registered, will return the existing ID
             *  @param name New event category name
             *  @return Non-zero id to event category
             */
        
            event_category_id_t RegisterEventCategory(const std::string& name);
         
            /** 
             * Queries for an event category ID by name.
             * @param name Event category name
             * @param create Whether will create category if missing (default true)
             * @return Non-zero id to event category, or illegal category if missing and create = false
             */
            
            event_category_id_t QueryEventCategory(const std::string& name, bool create = true);

            /** 
             * Queries for an event category name by ID
             *  @param category_id event category ID
             *  @return event category name, or empty string if event category not recognized
             */
            
            const std::string& QueryEventCategoryName(event_category_id_t category_id) const;


            /** 
             * Registers an event
             * Currently only for debugging purposes, not necessary to send the event in question
             * @param category_id Event category ID, should be registered first
             * @param event_id Event ID
             * @param name Event name
             */
        
            void RegisterEvent(event_category_id_t category_id, event_id_t event_id, const std::string& name);

            
            /**
             * Queries an event name by category & event ID
             */
            void QueryEventName(event_category_id_t category_id, event_id_t event_id) const;
             
            //! Sends an event
            /*! \param category_id Event category ID
                \param event_id Event ID
                \param data Pointer to event data structure (event-specific), can be 0 if not needed
                \return true if event was handled by some event handler
             */
            bool SendEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);
            
            //! Sends an event
            /*! \param category Event category name. Will be auto-registered if it does not exist
                \param event_id Event ID
                \param data Pointer to event data structure (event-specific), can be 0 if not needed
                \return true if event was handled by some event handler
             */
            bool SendEvent(const std::string& category, event_id_t event_id, IEventData* data);
            
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
            
            template <class T> void SendDelayedEvent(event_category_id_t category_id, event_id_t event_id, boost::shared_ptr<T> data, f64 delay = 0.0);
            

            //! Registers a module or component to the event subscriber list
            /*! Do not call while responding to an event! Note that it is ok to resubscribe your module to change priority.
                \param module Module to register
                \param priority Priority. Higher priority = gets called first
                \return true if successfully subscribed
             */
            
            template <typename T> bool RegisterEventSubscriber(T* subscriber, int priority);

            //! Unregisters a module or component from the subscriber tree
            /*! Do not call while responding to an event!
                @param module Module to unregister
                @return true if successfully unsubscribed
                @note if @p subscriber is a component it will also remove it from special event subscriber map!!!
             */
            
            template <typename T> bool UnregisterEventSubscriber(T* subscriber);
            
            //! Checks if module or component is registered as an event subscriber
            /*! \param module Module to check
                \return true if is registered
             */
            
            template <typename T> bool HasEventSubscriber(T* subscriber);
            
            //! Clears all delayed events. Called by the framework.
            /*! Called before unloading modules so that shared pointers left in the delayed event queue do not cause trouble
                (for example Ogre textures that would otherwise freed after Ogre uninit, leading to a crash)
             */
            
            void ClearDelayedEvents();
            
            //! Processes delayed events. Called by the framework.
            /*! \param frametime Time since last frame
             */ 
            void ProcessDelayedEvents(f64 frametime);
            
            
           
            //! Returns event category map
            const EventCategoryMap &GetEventCategoryMap() const { return event_category_map_; }
            
            //! Returns event map
            const EventMap &GetEventMap() const { return event_map_; }
             
            //! Returns next unused non-zero request tag for asset/resource request events
            /*! By having a global source for the tags there is no risk for collisions between
                different modules/subsystems.
             */
            request_tag_t GetNextRequestTag();

            /**
             * Registers component to listen only event which is defined by given @p category_id and @p event_id.
             * @param component is component which will get event. 
             * @param category_id is events category id. 
             * @param event_id is event id. 
             * @return true if registering was succesfull false if not. 
             * @note does not check that is component allready register through @p RegisterEventSubscriber(T* subscriber,int priority)
             */
            
            bool RegisterEventSubscriber(IComponent* component, event_category_id_t category_id, event_id_t event_id);

            /**
             * Unregister specialized events. 
             * @param component which will be unregistered.
             * @param category_id its category_id.
             * @param event_id its event_id.
             * @return true if unregister was successful 
             * @note If component is also register through RegisterEventSubscriber(T* subscriber,int priority), this function does not remove it from that lists
             */
            
            bool UnregisterEventSubscriber(IComponent* component, event_category_id_t category_id,event_id_t event_id);

        private:
           
           //! Event subscriber. Used internally by EventManager.
           template <typename T> class EventSubscriber 
           {
           public:
               EventSubscriber() : priority_(0), subscriber_(0) {}
               virtual ~EventSubscriber() { subscriber_ = 0; }
               
               T* subscriber_;
               int priority_;
              
               bool operator<(const EventSubscriber& rhs) const
               {
                    return priority_ > rhs.priority_;
               }
              
           }; 

           
           //! Delayed event. Used internally by EventManager.
           struct DelayedEvent
           {
                event_category_id_t category_id_;
                event_id_t event_id_;
                EventDataPtr data_;
                f64 delay_;
           };

          
            /** 
             * Sends event to a module in the subscriber vector
             * @param subscriber Which subscriber to send to
             * @param category_id Event category ID
             * @param event_id Event ID
             * @param data Pointer to event data structure (event-specific)
             * @return true if event handled and further subscribers should not be processed
             */
          
           template <typename T> bool SendEvent(const EventSubscriber<T>& subs, event_category_id_t category_id, event_id_t event_id, IEventData* data) const;

           template <typename T, typename U> bool AddSubscriber(T* subscriber, QList<U>& subscribers, int priority);
           
           template <typename T, typename U> bool RemoveSubscriber(T* subscriber, QList<U>& subscribers );

           template <typename T, typename U> bool EventSubscriberExist(T* subscriber, QList<U>& subscribers);


            //! Next event category ID that will be assigned
            event_category_id_t next_category_id_;
            
            //! Next free request tag to be used
            request_tag_t next_request_tag_;
            
            //! Map for assigned event category id's
            EventCategoryMap event_category_map_;
            
            //! Map for registered events by category
            EventMap event_map_;
            
            /// Module event subscribers
            QList<EventSubscriber<IModule > > module_subscribers_;

            /// Component event subscribers
            QList<EventSubscriber<IComponent > > component_subscribers_;
            
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

            QMap<QPair<event_category_id_t, event_id_t>, QList<IComponent* > > specialEvents_;
    };

}

#include "EventManager-templates.h"

#endif 