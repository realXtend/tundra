// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EventManager.h"
#include "Framework.h"
#include "EventDataInterface.h"
#include "ModuleManager.h"
#include "CoreException.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QThread>

#include <algorithm>

#include "MemoryLeakCheck.h"

namespace Foundation
{
   
    
    EventManager::EventManager(Framework *framework) : 
        framework_(framework),
        next_category_id_(1),
        next_request_tag_(1),
        main_thread_id_(QThread::currentThreadId())
    {
    }
    
    EventManager::~EventManager()
    {
    }
    
    event_category_id_t EventManager::RegisterEventCategory(const std::string& name)
    {
        if (event_category_map_.find(name) == event_category_map_.end())
        {
            event_category_map_[name] = next_category_id_;
            next_category_id_++;
            Foundation::RootLogDebug("Registered event category " + name);
        }
        else
        {
            Foundation::RootLogWarning("Event category " + name + " is already registered");
        }
        
        return event_category_map_[name];
    }
    
    event_category_id_t EventManager::QueryEventCategory(const std::string& name, bool create)
    {
        EventCategoryMap::const_iterator i = event_category_map_.find(name);
        if (i != event_category_map_.end())
            return i->second;
        
        if (create)
        {
            event_category_map_[name] = next_category_id_;
            next_category_id_++;
            Foundation::RootLogDebug("Registered event category " + name + " by query");
            return event_category_map_[name];
        }
        else
            return IllegalEventCategory;
    }
    
    const std::string& EventManager::QueryEventCategoryName(event_category_id_t category_id) const
    {
        EventCategoryMap::const_iterator i = event_category_map_.begin();
        static std::string empty;
        
        while (i != event_category_map_.end())
        {
            if (i->second == category_id)
                return i->first;
             
            ++i;
        }
        
        return empty;
    }
    
    void EventManager::RegisterEvent(event_category_id_t category_id, event_id_t event_id, const std::string& name)
    {
        if (!QueryEventCategoryName(category_id).length())
        {
            Foundation::RootLogError("Trying to register an event for yet unregistered category");
            return;
        }
        
        if (event_map_[category_id].find(event_id) != event_map_[category_id].end())
            Foundation::RootLogWarning("Overwriting already registered event with " + name);
        else
            Foundation::RootLogDebug("Registering event " + name);

        event_map_[category_id][event_id] = name;
    }
    
    bool EventManager::SendEvent(event_category_id_t category_id, event_id_t event_id, EventDataInterface* data)
    {
        if (QThread::currentThreadId() != main_thread_id_)
        {
            Foundation::RootLogError("Tried to send an immediate event (using SendEvent) from a thread that is not the main thread. Use SendDelayedEvent() instead.");
            throw Exception("Tried to send an immediate event (using SendEvent) from a thread that is not the main thread. Use SendDelayedEvent() instead.");
        }
        
        // Do not send messages after exit
        if (framework_->IsExiting())
            return false;
        if (category_id == IllegalEventCategory)
        {
            Foundation::RootLogWarning("Attempted to send event with illegal category");
            return false;
        }
        
        // Send event in priority order, until someone returns true
        for (unsigned i = 0; i < module_subscribers_.size(); ++i)
        {
            if (SendEvent(module_subscribers_[i], category_id, event_id, data))
                return true;
        }
        
        // After that send events to components

        for (unsigned i = 0; i < component_subscribers_.size(); ++i)
        {
            if (SendEvent(component_subscribers_[i], category_id, event_id, data))
                return true;
        }

        // Then go through 
        QPair<event_category_id_t, event_id_t> group = qMakePair<event_category_id_t, event_id_t>(category_id, event_id);
       
        if ( specialEvents_.contains(group) )
        {
            QList<ComponentInterface* > lst = specialEvents_[group];
            for ( int i = 0; i < lst.size(); ++i)
            {
                EventSubscriber<ComponentInterface> subs;
                subs.subscriber_ = lst[i];

                if (SendEvent(subs, category_id, event_id, data))
                    return true;

            }
        }

        return false;



    }
    
    bool EventManager::SendEvent(const std::string& category, event_id_t event_id, EventDataInterface* data)
    {
        return SendEvent(QueryEventCategory(category), event_id, data);
    }
    
    void EventManager::SendDelayedEvent(event_category_id_t category_id, event_id_t event_id, EventDataPtr data, f64 delay)
    {
        MutexLock lock(delayed_events_mutex_);
        
        // Do not send messages after exit
        if (framework_->IsExiting())
            return;
            
        if (category_id == IllegalEventCategory)
        {
            Foundation::RootLogWarning("Attempted to send delayed event with illegal category");
            return;
        }    
        
        DelayedEvent new_delayed_event;
        new_delayed_event.category_id_ = category_id;
        new_delayed_event.event_id_ = event_id;
        new_delayed_event.data_ = data;
        new_delayed_event.delay_ = delay;
        
        new_delayed_events_.push_back(new_delayed_event);
    }
    

    bool EventManager::RegisterEventSubscriber(ComponentInterface* component, event_category_id_t category_id, event_id_t event_id)
    {
       
        QPair<event_category_id_t, event_id_t> group = qMakePair<event_category_id_t, event_id_t>(category_id, event_id);
       
        if ( specialEvents_.contains(group) )
        {
            QList<ComponentInterface* > lst = specialEvents_[group];
            lst.append(component);
        }
        else
        {
            QList<ComponentInterface* > lst;
            lst.append(component);
            specialEvents_.insert(group,lst); 
        }

        return true;
    }

    bool EventManager::UnregisterEventSubscriber(ComponentInterface* component, event_category_id_t category_id,event_id_t event_id)
     {
       QPair<event_category_id_t, event_id_t> group = qMakePair<event_category_id_t, event_id_t>(category_id, event_id);
       if ( specialEvents_.contains(group) )
       {
            specialEvents_.remove(group);
            return true;
       }

       return false;
     }

   
    request_tag_t EventManager::GetNextRequestTag()
    {
        if (next_request_tag_ == 0) 
            ++next_request_tag_; // Never use 0
        return next_request_tag_++;
    }    
    
    void EventManager::ClearDelayedEvents()
    {
        MutexLock lock(delayed_events_mutex_);
        delayed_events_.clear();
        new_delayed_events_.clear();
    }
    
    void EventManager::ProcessDelayedEvents(f64 frametime)
    {
        {
            MutexLock lock(delayed_events_mutex_);
            delayed_events_.insert(delayed_events_.end(), new_delayed_events_.begin(), new_delayed_events_.end());
            new_delayed_events_.clear();
        }
        
        DelayedEventVector::iterator i = delayed_events_.begin();
        
        while (i != delayed_events_.end())
        {
            if (i->delay_ <= 0.0)
            {
                SendEvent(i->category_id_, i->event_id_, i->data_.get());
                i = delayed_events_.erase(i);
            }
            else
            {   
                i->delay_ -= frametime;
                ++i;
            }
        }
    }
}
