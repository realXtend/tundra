#include "StableHeaders.h"
#include "Framework.h"
#include "EventDataInterface.h"
#include "EventManager.h"
#include "ModuleManager.h"
#include "CoreException.h"

#include <QDomDocument>
#include <QDomElement>
#include <QFile>
#include <QThread>

#include <algorithm>

namespace Foundation
{
    EventManager::EventManager(Framework *framework) : 
        framework_(framework),
        next_category_id_(1),
        next_request_tag_(1),
        event_subscriber_root_(EventSubscriberPtr(new EventSubscriber())),
        main_thread_id_(QThread::currentThreadId())
    {
    }

    EventManager::~EventManager()
    {
        event_subscriber_root_.reset();
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
    
    event_category_id_t EventManager::QueryEventCategory(const std::string& name) const
    {
        EventCategoryMap::const_iterator i = event_category_map_.find(name);
        if (i != event_category_map_.end())
            return i->second;
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
    
    bool EventManager::SendEvent(event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) const
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
            
        return SendEvent(event_subscriber_root_.get(), category_id, event_id, data);
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
    
    bool EventManager::SendEvent(EventSubscriber* node, event_category_id_t category_id, event_id_t event_id, EventDataInterface* data) const
    {
        if (ModuleSharedPtr module = node->module_.lock())
        {
            if (module->HandleEvent(category_id, event_id, data))
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
    
    bool EventManager::RegisterEventSubscriber(ModuleWeakPtr module_, int priority, ModuleWeakPtr parent_)
    {
        ModuleSharedPtr modulePtr = module_.lock();
        ModuleSharedPtr parentPtr = parent_.lock();
        ModuleInterface *module = modulePtr.get();
        ModuleInterface *parent = parentPtr.get();
        assert (module);
        
        if (FindNodeWithModule(event_subscriber_root_.get(), module))
        {
            Foundation::RootLogWarning(module->Name() + " is already added as event subscriber");
            return false;
        }
        
        EventSubscriber* node = FindNodeWithModule(event_subscriber_root_.get(), parent);
        if (!node)
        {
            if (parent)
                Foundation::RootLogWarning("Could not add module " + module->Name() + " as event subscriber, parent module " + parent->Name() + " not found");
            
            return false;
        }
        
        EventSubscriberPtr new_node = EventSubscriberPtr(new EventSubscriber());
        new_node->module_ = module_;
        new_node->module_name_ = module->Name();
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
            Foundation::RootLogWarning("Could not remove event subscriber " + module->Name() + ", not found");
            return false;
        }
        
        EventSubscriberVector::iterator i = node->children_.begin();
        while (i != node->children_.end())
        {
            if ((*i)->module_.lock().get() == module)
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
        
        return (FindNodeWithChild(event_subscriber_root_.get(), module) != 0);
    }

    void EventManager::ValidateEventSubscriberTree()
    {
        ValidateEventSubscriberTree(event_subscriber_root_.get());
    }

    void EventManager::ValidateEventSubscriberTree(EventSubscriber* node)
    {
        if (!node->module_name_.empty())
            node->module_ = framework_->GetModuleManager()->GetModule(node->module_name_);
        else
            node->module_ = ModuleWeakPtr();
            
        EventSubscriberVector::const_iterator i = node->children_.begin();
        while (i != node->children_.end())
        {
            ValidateEventSubscriberTree((*i).get());
            ++i;
        }
    }

    EventManager::EventSubscriber* EventManager::FindNodeWithModule(EventSubscriber* node, ModuleInterface* module) const
    {
        if (node->module_.lock().get() == module)
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
            if ((*i)->module_.lock().get() == module)
                return node;
                
            EventSubscriber* result = FindNodeWithChild((*i).get(), module);
            if (result)
                return result;
                
            ++i;
        }
        
        return 0;
    }

    void EventManager::LoadEventSubscriberTree(const std::string& filename)
    {
        PROFILE(EventManager_LoadEventSubscriberTree);

        Foundation::RootLogDebug("Loading event subscriber tree with " + filename);

        QFile file(filename.c_str());
        if (!file.open(QIODevice::ReadOnly))
        {
            Foundation::RootLogError("Could not open subscriber tree file " + filename);
            return;
        }
        QDomDocument doc("Subscribers");
        if (!doc.setContent(&file))
        {
            file.close();
            Foundation::RootLogError("Could not load subscriber tree file " + filename);
            return;
        }
        file.close();

        QDomElement elem = doc.firstChildElement();
        if (!elem.isNull())
            BuildTreeFromNode(elem, "");
    }
    
    void EventManager::BuildTreeFromNode(QDomElement& elem, const std::string parent_name)
    {
        while (!elem.isNull())
        {
            std::string new_parent_name = parent_name;
            
            std::string module_name = elem.attribute("module").toStdString();
            std::string priority_str = elem.attribute("priority").toStdString();
            
            if ((!module_name.empty()) && (!priority_str.empty()))
            {
                int priority = boost::lexical_cast<int>(priority_str);
                
                new_parent_name = module_name;
                
                ModuleWeakPtr module_weak(framework_->GetModuleManager()->GetModule(module_name));
                ModuleSharedPtr module (module_weak.lock());
                if (module)
                {
                    if (parent_name.empty())
                    {
                        RegisterEventSubscriber(module, priority, ModuleWeakPtr());
                    }
                    else
                    {
                        ModuleWeakPtr parent = framework_->GetModuleManager()->GetModule(parent_name);
                        if (parent.lock().get())
                        {
                            RegisterEventSubscriber(module, priority, parent);
                        }
                        else
                        {
                            Foundation::RootLogWarning("Parent module " + parent_name + " not found for module " + module_name);
                        }
                    }
                }
                else
                {
                    Foundation::RootLogWarning("Module " + module_name + " not found");
                }
            }
            
            QDomElement childElem = elem.firstChildElement();
            if (!childElem.isNull())
            {
                BuildTreeFromNode(childElem, new_parent_name);
            }
            
            elem = elem.nextSiblingElement();
        }
    }
    
    request_tag_t EventManager::GetNextRequestTag()
    {
        if (next_request_tag_ == 0) 
            ++next_request_tag_; // Never use 0
        return next_request_tag_++;
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
