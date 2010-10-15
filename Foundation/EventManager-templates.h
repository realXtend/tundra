// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_EventManager_h
# error "Never use <EventManager-templates.h> directly; include <EventManager.h> instead."
#endif

namespace Foundation
{
    template <class T> void EventManager::SendDelayedEvent(event_category_id_t category_id, event_id_t event_id, boost::shared_ptr<T> data, f64 delay)
    {
       SendDelayedEvent(category_id, event_id, boost::dynamic_pointer_cast<IEventData>(data), delay);
    }

    template <typename T, typename U> bool EventManager::AddSubscriber(T* subscriber, QList<U>& subscribers, int priority)
    {
        for(int i = 0; i < subscribers.size(); ++i)
            // If subscriber found, just readjust the priority
            if (subscribers[i].subscriber_ == subscriber)
            {
                subscribers[i].priority_ = priority;
                qSort(subscribers.begin(), subscribers.end());
                return true;
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

        IModule* module = dynamic_cast<IModule* >(subscriber);
        if ( module != 0)
            return AddSubscriber(module, module_subscribers_, priority);

        IComponent* component = dynamic_cast<IComponent* >(subscriber);
        if ( component != 0 )
            return AddSubscriber(component, component_subscribers_, priority);

        return false;
    }

    template <typename T, typename U> bool EventManager::RemoveSubscriber(T* subscriber, QList<U>& subscribers )
    {
        for (int i = 0; i < subscribers.size(); ++i)
            if (subscribers[i].subscriber_ == subscriber)
            {
                subscribers.erase(subscribers.begin() + i);
                return true;
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

        IModule* module = dynamic_cast<IModule* >(subscriber);
        if ( module != 0)
            return RemoveSubscriber(module, module_subscribers_);

        IComponent* component = dynamic_cast<IComponent* >(subscriber);

        if ( component != 0 )
        {
            bool ret = RemoveSubscriber(component, component_subscribers_);
            bool ret2 = false;

            QMap<QPair<event_category_id_t, event_id_t>, QList<IComponent* > >::iterator iter;
            for (iter = specialEvents_.begin(); iter != specialEvents_.end();)
            {
                QList<IComponent* >& lst = specialEvents_[iter.key()];

                for(int i = lst.size() - 1; i >= 0; --i)
                {
                    if ( lst[i] == component )
                    {
                        lst.removeAt(i);
                        ret2 = true;
                    }
                }

                if (lst.isEmpty())
                    iter = specialEvents_.erase(iter);
                else
                    ++iter;
            }

            return (ret || ret2);
        }

        return false;
    }

    template <typename T, typename U> bool EventManager::EventSubscriberExist(T* subscriber, QList<U>& subscribers)
    {
        for (unsigned i = 0; i < subscribers.size(); ++i)
            if (subscribers[i].subscriber_ == subscriber)
                return true;

        return false;
    }

    template <typename T> bool EventManager::HasEventSubscriber(T* subscriber)
    {
        if (!subscriber)
        {
            RootLogError("Tried to check null event subscriber");
            return false;
        }

        IModule* module = dynamic_cast<IModule* >(subscriber);
        if ( module != 0)
            return EventSubscriberExist(module, module_subscribers_);

        IComponent* component = dynamic_cast<IComponent* >(subscriber);
        if ( component != 0 )
            return EventSubscriberExist(component, component_subscribers_);

        return false;
     }

    template <typename T> bool EventManager::SendEvent(const EventSubscriber<T>& subs, event_category_id_t category_id, event_id_t event_id, IEventData* data) const
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
