// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <Poco/UnicodeConverter.h>
#include <Poco/DOM/DOMParser.h>
#include <Poco/DOM/Element.h>
#include <Poco/DOM/Attr.h>
#include <Poco/DOM/NamedNodeMap.h>
#include <Poco/DOM/AutoPtr.h>
#include <Poco/SAX/InputSource.h>
#include <Poco/SAX/AttributesImpl.h>

#include "Mapper.h"
#include "InputEvents.h"

namespace Input
{
    Mapper::Mapper(InputModuleOIS *module) : 
        module_(module)
    {
        assert (module_);

        const Foundation::ConfigurationManager &config = module_->GetFramework()->GetDefaultConfig();
            
        std::wstring input_mappings_w = Core::ToWString(config.DeclareSetting("Input", "input_mappings_file", std::string("input_map.xml")));
        std::wstring app_data = module_->GetFramework()->GetPlatform()->GetApplicationDataDirectoryW();

        input_mappings_w = app_data + L"/" + input_mappings_w;
        

        std::string input_mappings;
        Poco::UnicodeConverter::toUTF8(input_mappings_w, input_mappings);

        SetDefaultMappings();
        LoadInputMappings(input_mappings);
    }

    Mapper::~Mapper()
    {
    }

    void Mapper::LoadInputMappings(const std::string &file)
    {
        if (boost::filesystem::exists(file) == false)
        {
            InputModuleOIS::LogInfo("Input mappings file not found, using default mappings.");

            ExportDefaults(file);
        } else
        {
            InputModuleOIS::LogInfo("Loading input mappings from file " + file + "...");
            try
            {
                Poco::XML::InputSource source(file);
                Poco::XML::DOMParser parser;
                Poco::XML::AutoPtr<Poco::XML::Document> document = parser.parse(&source);
                
                if (!document.isNull())
                {
                    Poco::XML::Node* node = document->firstChild();
                    if (node)
                    {
                        LoadInputMappings(node);
                    }
                }
                else
                {
                    throw Core::Exception("Failed to parse xml document.");
                }
            } catch (std::exception &e)
            {
                InputModuleOIS::LogInfo(e.what());
                InputModuleOIS::LogInfo("Failed to parse input mappings file, using default mappings.");
            }
        }
    }

    void Mapper::LoadInputMappings(const Poco::XML::Node* node)
    {
        node = node->firstChild();
        if (!node)
            throw Core::Exception("Child node not found for root node.");

        while (node)
        {
            const Poco::XML::Node* current_node = node;
            node = node->nextSibling();    

            int type = current_node->nodeType();
            if (current_node->nodeType() == Poco::XML::Node::ELEMENT_NODE)
            {
                Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> attributes = current_node->attributes();
                if (attributes.isNull())
                    throw Core::Exception("Missing attributes.");

                Poco::XML::Attr* action_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("value"));
                Poco::XML::Attr* modifier_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("modifier"));
                Poco::XML::Attr* key_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("key"));

                if (!action_attr)
                    throw Core::Exception("Missing attribute 'value'.");
                if (!modifier_attr)
                    throw Core::Exception("Missing attribute 'modifier'.");
                if (!key_attr)
                    throw Core::Exception("Missing attribute 'key'.");

                Core::event_id_t event_id = Core::ParseString<Core::event_id_t>(action_attr->getValue());
                int modifier = Core::ParseString<int>(modifier_attr->getValue());
                int key = Core::ParseString<int>(key_attr->getValue());

                module_->RegisterUnbufferedKeyEvent(static_cast<OIS::KeyCode>(key), event_id, event_id + 1, modifier);

            }
        }
    }

    void Mapper::ExportDefaults(const std::string &file)
    {
        InputModuleOIS::LogInfo("Exporting default input mappings to file " + file + "...");

        std::fstream file_op(file.c_str(), std::ios::out);

        Poco::XML::XMLWriter writer(file_op, Poco::XML::XMLWriter::CANONICAL);
	    writer.startDocument();
	    writer.startElement("", "", "input");

        const InputModuleOIS::KeyEventInfoVector &events = module_->GetRegisteredKeyEvents();
        for ( InputModuleOIS::KeyEventInfoVector::const_iterator info = events.begin() ; 
              info != events.end() ;
              ++info )
        {
            Poco::XML::AttributesImpl attrs;
            attrs.addAttribute("", "", "value", "CDATA", Core::ToString(info->pressed_event_id_));
	        attrs.addAttribute("", "", "modifier", "CDATA", Core::ToString(info->modifier_));
            attrs.addAttribute("", "", "key", "CDATA", Core::ToString(info->key_));
	        writer.emptyElement("", "", "action", attrs);
        }
        writer.endElement("", "", "input");
	    writer.endDocument();
    }

    void Mapper::SetDefaultMappings()
    {
        module_->RegisterUnbufferedKeyEvent(OIS::KC_W,       Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(OIS::KC_S,       Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(OIS::KC_A,       Events::MOVE_LEFT_PRESSED,      Events::MOVE_LEFT_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(OIS::KC_D,       Events::MOVE_RIGHT_PRESSED,     Events::MOVE_RIGHT_RELEASED,   0);
        module_->RegisterUnbufferedKeyEvent(OIS::KC_UP,      Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(OIS::KC_DOWN,    Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(OIS::KC_LEFT,    Events::MOVE_LEFT_PRESSED,      Events::MOVE_LEFT_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(OIS::KC_RIGHT,   Events::MOVE_RIGHT_PRESSED,     Events::MOVE_RIGHT_RELEASED,   0);
    }
}

