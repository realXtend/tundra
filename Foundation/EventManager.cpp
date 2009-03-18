#include "StableHeaders.h"
#include "Framework.h"
#include "EventDataInterface.h"
#include "EventManager.h"
#include "ModuleManager.h"

#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Attr.h"
#include "Poco/DOM/NamedNodeMap.h"
#include "Poco/SAX/InputSource.h"

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
            Foundation::RootLogWarning("Event category " + name + " is already registered");
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
            Foundation::RootLogWarning("Could not remove event subscriber " + module->Name() + ", not found");
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

    void EventManager::LoadEventSubscriberTree(const std::string& filename)
    {
        Foundation::RootLogInfo("Loading event subscriber tree from " + filename);
        
        try
        {
            Poco::XML::InputSource source(filename);
            Poco::XML::DOMParser parser;
            Poco::XML::Document* document = parser.parse(&source);

            Poco::XML::Node* node = document->firstChild();
            if (node)
            {
                BuildTreeFromNode(node, "");
            }
        }
        catch (Poco::Exception& e)
        {
            Foundation::RootLogError("Could not load event subscriber tree from " + filename + ": " + e.what());
        }
    }
    
    void EventManager::BuildTreeFromNode(Poco::XML::Node* node, const std::string parent_name)
    {
        while (node)
        {
            std::string new_parent_name = parent_name;
            
            Poco::XML::NamedNodeMap* attributes = node->attributes();
            if (attributes)
            {
                Poco::XML::Attr* module_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("module"));
                Poco::XML::Attr* priority_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("priority"));

                if ((module_attr) && (priority_attr))
                {
                    const std::string& module_name = module_attr->getValue();
                    int priority = boost::lexical_cast<int>(priority_attr->getValue());
                    
                    new_parent_name = module_name;
                    
                    ModuleInterface* module = framework_->GetModuleManager()->GetModule(module_name);
                    if (module)
                    {
                        if (parent_name.empty())
                        {
                            RegisterEventSubscriber(module, priority, NULL);
                        }
                        else
                        {
                            ModuleInterface* parent = framework_->GetModuleManager()->GetModule(parent_name);
                            if (parent)
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
            }
            
            if (node->firstChild())
            {
                BuildTreeFromNode(node->firstChild(), new_parent_name);
            }
            
            node = node->nextSibling();
        }
    }
}