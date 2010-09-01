#ifndef incl_Foundation_EventManager_h
# error "Never use <EventManager-templates.h> directly; include <EventManager.h> instead."
#endif

namespace Foundation
{
  
    template <class T> void EventManager::SendDelayedEvent(event_category_id_t category_id, event_id_t event_id, boost::shared_ptr<T> data, f64 delay)
    {
       SendDelayedEvent(category_id, event_id, boost::dynamic_pointer_cast<EventDataInterface>(data), delay);     
    }

    template <typename T, typename U> bool EventManager::AddSubscriber(T* subscriber, QList<U>& subscribers, int priority)
    {
        
        for (unsigned int i = 0; i < subscribers.size(); ++i)
        {
          // If subscriber found, just readjust the priority
 
          if (subscribers[i].subscriber_ == subscriber)
            {
                subscribers[i].priority_ = priority;
                qSort(subscribers.begin(), subscribers.end());
                return true;
            }
         }   
         
          EventSubscriber<T> new_subscriber;
          new_subscriber.subscriber_ = subscriber;
          
          new_subscriber.priority_ = priority;
          subscribers.append(new_subscriber);
          qSort(subscribers.begin(), subscribers.end());
          
          return true;
      
    }

    template <typename T> bool EventManager::RegisterEventSubscriber(T* subscriber, int priority)
    {
        if (!subscriber)
        {
            RootLogError("Tried to register null as event subscriber");
            return false;
        }
        
        ModuleInterface* module = dynamic_cast<ModuleInterface* >(subscriber);
        
        if ( module != 0)
            return AddSubscriber(module, module_subscribers_, priority);

        ComponentInterface* component = dynamic_cast<ComponentInterface* >(subscriber);

        if ( component != 0 )
            return AddSubscriber(component, component_subscribers_, priority);
                    
        return false;
    }

    template <typename T, typename U> bool EventManager::RemoveSubscriber(T* subscriber, QList<U>& subscribers )
    {
        for (unsigned i = 0; i < subscribers.size(); ++i)
        {
            if (subscribers[i].subscriber_ == subscriber)
            {
                subscribers.erase(subscribers.begin() + i);
                return true;
            }
        }
        return false;
    }

    template <typename T> bool EventManager::UnregisterEventSubscriber(T* subscriber)
    {
        if (!subscriber)
        {
            RootLogError("Tried to remove null as event subscriber");
            return false;
        }
            
        ModuleInterface* module = dynamic_cast<ModuleInterface* >(subscriber);
        
        if ( module != 0)
            return RemoveSubscriber(module, module_subscribers_);

        ComponentInterface* component = dynamic_cast<ComponentInterface* >(subscriber);

        if ( component != 0 )
            return RemoveSubscriber(component, component_subscribers_);
                    
        return false;

    }

    template <typename T, typename U> bool EventManager::EventSubscriberExist(T* subscriber, QList<U>& subscribers)
    {
        for (unsigned i = 0; i < subscribers.size(); ++i)
        {
            if (subscribers[i].subscriber_ == subscriber)
                return true;
        }
        
        return false;
    }

    template <typename T> bool EventManager::HasEventSubscriber(T* subscriber)
     {
        if (!subscriber)
        {
            RootLogError("Tried to check null event subscriber");
            return false;
        }
        
        ModuleInterface* module = dynamic_cast<ModuleInterface* >(subscriber);
        
        if ( module != 0)
            return EventSubscriberExist(module, module_subscribers_);

        ComponentInterface* component = dynamic_cast<ComponentInterface* >(subscriber);

        if ( component != 0 )
            return EventSubscriberExist(component, component_subscribers_);
                    
        return false;
          
       
     }

    template <typename T> bool EventManager::SendEvent(const EventSubscriber<T>& subs, event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) const
     {
        T* subscriber = subs.subscriber_;
        if (subscriber)
        {
            try
            {
                return subscriber->HandleEvent(category_id, event_id, data);
            }
            catch(const std::exception &/*e*/)
            {
                //std::cout << "HandleEvent caught an exception inside module " << module->Name() << ": " << (e.what() ? e.what() : "(null)") << std::endl;
                //RootLogCritical(std::string("HandleEvent caught an exception inside module " + module->Name() + ": " + (e.what() ? e.what() : "(null)")));
                throw;
            }
            catch(...)
            {
                //std::cout << "HandleEvent caught an unknown exception inside module " << module->Name() << std::endl;
                //RootLogCritical(std::string("HandleEvent caught an unknown exception inside module " + module->Name()));
                throw;
            }
        }
        
        return false;

     }


}