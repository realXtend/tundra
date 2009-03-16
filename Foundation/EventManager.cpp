#include "StableHeaders.h"
#include "Framework.h"
#include "EventDataInterface.h"
#include "EventManager.h"
#include "ModuleInterface.h"

#include <algorithm>

namespace Foundation
{
    EventManager::EventManager(Framework *framework) : 
        framework_(framework),
        next_category_id_(1),
        event_subscriber_root_(EventSubscriberPtr(new EventSubscriber()))
    {
    }

    EventManager::~EventManager()
    {
        event_subscriber_root_.reset();
    }
    
    Core::event_category_id_t EventManager::RegisterEventCategory(const std::string& name)
    {
        if (event_category_map_.find(name) == event_category_map_.end())
        {
            event_category_map_[name] = next_category_id_;
            next_category_id_++;
        }
        else
        {
            throw Core::Exception((std::string("Event category " + name + " already registered.")).c_str());
        }
        
        return event_category_map_[name];
    }
    
    Core::event_category_id_t EventManager::QueryEventCategory(const std::string& name) const
    {
        EventCategoryMap::const_iterator i = event_category_map_.find(name);
        if (i != event_category_map_.end())
            return i->second;
        else 
            return 0;
    }
    
    const std::string& EventManager::QueryEventCategoryName(Core::event_category_id_t id) const
    {
        EventCategoryMap::const_iterator i = event_category_map_.begin();
        static std::string empty;
        
        while (i != event_category_map_.end())
        {
            if (i->second == id)
                return i->first;
             
            ++i;
        }
        
        return empty;
    }
    
    void EventManager::SendEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) const
    {
        SendEvent(event_subscriber_root_.get(), category_id, event_id, data);
    }
    
    bool EventManager::SendEvent(EventSubscriber* node, Core::event_category_id_t category_id, Core::event_id_t event_id, EventDataInterface* data) const
    {
        if (node->module_)
        {
            if (node->module_->HandleEvent(category_id, event_id, data))
                return true;
        }
        
        EventSubscriberVector::const_iterator i = node->children_.begin();
        while (i != node->children_.end())
        {
            if (SendEvent((*i).get(), category_id, event_id, data))
                return true;
                
            ++i;
        }
        
        return false;
    }
    
    bool ComparePriority(EventManager::EventSubscriberPtr const& e1, EventManager::EventSubscriberPtr const& e2)
    {
        return e1.get()->priority_ < e2.get()->priority_;
    }
    
    bool EventManager::RegisterEventSubscriber(ModuleInterface* module, int priority, ModuleInterface* parent)
    {
        assert (module);
        
        if (FindNodeWithModule(event_subscriber_root_.get(), module))
        {
            Foundation::RootLogError(module->Name() + " is already added as event subscriber");
            return false;
        }
        
        EventSubscriber* node = FindNodeWithModule(event_subscriber_root_.get(), parent);
        if (!node)
        {
            if (parent)
                Foundation::RootLogError("Could not add module " + module->Name() + " as event subscriber, parent module " + parent->Name() + " not found");
            
            return false;
        }
        
        EventSubscriberPtr new_node = EventSubscriberPtr(new EventSubscriber());
        new_node->module_ = module;
        new_node->priority_ = priority;
        node->children_.push_back(new_node);
        std::sort(node->children_.rbegin(), node->children_.rend(), ComparePriority);
        return true;
    }
    
    bool EventManager::UnregisterEventSubscriber(ModuleInterface* module)
    {
        assert (module);
        
        EventSubscriber* node = FindNodeWithChild(event_subscriber_root_.get(), module);
        if (!node)
        {
            Foundation::RootLogError("Could not remove event subscriber " + module->Name() + ", not found");
            return false;
        }
        
        EventSubscriberVector::iterator i = node->children_.begin();
        while (i != node->children_.end())
        {
            if ((*i)->module_ == module)
            {
                node->children_.erase(i);
                return true;
            }
            
            ++i;
        }
        
        return false; // should not happen
    }
    
    bool EventManager::HasEventSubscriber(ModuleInterface* module)
    {
        assert (module);
        
        return (FindNodeWithChild(event_subscriber_root_.get(), module) != NULL);
    }

    EventManager::EventSubscriber* EventManager::FindNodeWithModule(EventSubscriber* node, ModuleInterface* module) const
    {
        if (node->module_ == module)
            return node;
            
        EventSubscriberVector::const_iterator i = node->children_.begin();
        while (i != node->children_.end())
        {
            EventSubscriber* result = FindNodeWithModule((*i).get(), module);
            if (result) 
                return result;
                
            ++i;
        }
        
        return 0;
    }
    
    EventManager::EventSubscriber* EventManager::FindNodeWithChild(EventSubscriber* node, ModuleInterface* module) const
    {
        EventSubscriberVector::const_iterator i = node->children_.begin();
        while (i != node->children_.end())
        {
            if ((*i)->module_ == module)
                return node;
                
            EventSubscriber* result = FindNodeWithChild((*i).get(), module);
            if (result)
                return result;
                
            ++i;
        }
        
        return 0;
    }
    
}