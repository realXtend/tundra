// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"

#include <Poco/Path.h>
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
            
        std::string input_mappings = config.DeclareSetting("Input", "input_mappings_file", std::string("input_map.xml"));
        std::string app_data = module_->GetFramework()->GetPlatform()->GetApplicationDataDirectory();

        input_mappings = app_data + "/" + input_mappings;

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

            Export(file);
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
                    throw Exception("Failed to parse xml document.");
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
            throw Exception("Child node not found for root node.");

        while (node)
        {
            const Poco::XML::Node* current_node = node;
            node = node->nextSibling();    

            int type = current_node->nodeType();
            if (current_node->nodeType() == Poco::XML::Node::ELEMENT_NODE)
            {
                if (current_node->nodeName() == "action")
                {
                    Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> attributes = current_node->attributes();
                    if (attributes.isNull())
                        throw Exception("Missing attributes.");

                    Poco::XML::Attr* state_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("state"));
                    Poco::XML::Attr* event_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("start_event"));
                    Poco::XML::Attr* end_event_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("end_event"));
                    Poco::XML::Attr* modifier_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("modifier"));
                    Poco::XML::Attr* key_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("key"));

                    if (!state_attr)
                        throw Exception("Missing attribute 'state'.");
                    if (!event_attr)
                        throw Exception("Missing attribute 'start_event'.");
                    if (!end_event_attr)
                        throw Exception("Missing attribute 'end_event'.");
                    if (!modifier_attr)
                        throw Exception("Missing attribute 'modifier'.");
                    if (!key_attr)
                        throw Exception("Missing attribute 'key'.");

                    Input::State state = static_cast<Input::State>(ParseString<int>(state_attr->getValue()));
                    event_id_t start_event_id = ParseString<event_id_t>(event_attr->getValue());
                    event_id_t end_event_id = ParseString<event_id_t>(end_event_attr->getValue());
                    int modifier = ParseString<int>(modifier_attr->getValue());
                    int key = ParseString<int>(key_attr->getValue());

                    module_->RegisterUnbufferedKeyEvent(state, static_cast<OIS::KeyCode>(key), start_event_id, end_event_id, modifier);
                } else if (current_node->nodeName() == "action_slider")
                {
                    Poco::XML::AutoPtr<Poco::XML::NamedNodeMap> attributes = current_node->attributes();
                    if (attributes.isNull())
                        throw Exception("Missing attributes.");

                    Poco::XML::Attr* state_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("state"));
                    Poco::XML::Attr* event_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("start_event"));
                    Poco::XML::Attr* end_event_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("end_event"));
                    Poco::XML::Attr* modifier_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("modifier"));
                    Poco::XML::Attr* key_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("button"));
                    Poco::XML::Attr* type_attr = static_cast<Poco::XML::Attr*>(attributes->getNamedItem("type"));

                    if (!state_attr)
                        throw Exception("Missing attribute 'state'.");
                    if (!event_attr)
                        throw Exception("Missing attribute 'start_event'.");
                    if (!end_event_attr)
                        throw Exception("Missing attribute 'end_event'.");
                    if (!modifier_attr)
                        throw Exception("Missing attribute 'modifier'.");
                    if (!key_attr)
                        throw Exception("Missing attribute 'button'.");
                    if (!type_attr)
                        throw Exception("Missing attribute 'type'.");

                    Input::State state = static_cast<Input::State>(ParseString<int>(state_attr->getValue()));
                    event_id_t start_event_id = ParseString<event_id_t>(event_attr->getValue());
                    event_id_t end_event_id = ParseString<event_id_t>(end_event_attr->getValue());
                    int modifier = ParseString<int>(modifier_attr->getValue());
                    int button = ParseString<int>(key_attr->getValue());
                    Input::Slider type = static_cast<Input::Slider>(ParseString<int>(type_attr->getValue()));

                    module_->RegisterSliderEvent(state, type, start_event_id, end_event_id, button, modifier);
                }
            }
        }
    }

    void Mapper::Export(const std::string &file)
    {
        InputModuleOIS::LogInfo("Exporting default input mappings to file " + file + "...");

        std::fstream file_op(file.c_str(), std::ios::out);

        Poco::XML::XMLWriter writer(file_op, Poco::XML::XMLWriter::CANONICAL);
	    writer.startDocument();
	    writer.startElement("", "", "input");

        {
            const InputModuleOIS::KeyEventInfoMap &events = module_->GetRegisteredKeyEvents();
            for ( InputModuleOIS::KeyEventInfoMap::const_iterator state = events.begin() ; 
                  state != events.end() ;
                  ++state )
            {
                for ( InputModuleOIS::KeyEventInfoVector::const_iterator info = state->second.begin() ; 
                  info != state->second.end() ;
                  ++info )
                {
                    Poco::XML::AttributesImpl attrs;
                    attrs.addAttribute("", "", "state", "CDATA", ToString(static_cast<int>(state->first)));
                    attrs.addAttribute("", "", "start_event", "CDATA", ToString(info->pressed_event_id_));
                    attrs.addAttribute("", "", "end_event", "CDATA", ToString(info->released_event_id_));
	                attrs.addAttribute("", "", "modifier", "CDATA", ToString(info->modifier_));
                    attrs.addAttribute("", "", "key", "CDATA", ToString(info->key_));
	                writer.emptyElement("", "", "action", attrs);
                }
            }
        }

        {
            const InputModuleOIS::SliderInfoMap &events = module_->GetRegisteredSliderEvents();
            for ( InputModuleOIS::SliderInfoMap::const_iterator state = events.begin() ; 
                  state != events.end() ;
                  ++state )
            {
                for ( InputModuleOIS::SliderInfoVector::const_iterator info = state->second.begin() ; 
                      info != state->second.end() ;
                      ++info )
                {
                    Poco::XML::AttributesImpl attrs;
                    attrs.addAttribute("", "", "state", "CDATA", ToString(static_cast<int>(state->first)));
                    attrs.addAttribute("", "", "start_event", "CDATA", ToString(info->dragged_event_));
                    attrs.addAttribute("", "", "end_event", "CDATA", ToString(info->stopped_event_));
	                attrs.addAttribute("", "", "modifier", "CDATA", ToString(info->modifier_));
                    attrs.addAttribute("", "", "button", "CDATA", ToString(info->button_));
                    attrs.addAttribute("", "", "type", "CDATA", ToString(info->slider_));
	                writer.emptyElement("", "", "action_slider", attrs);
                }
            }
        }
        writer.endElement("", "", "input");
	    writer.endDocument();
    }

    void Mapper::SetDefaultMappings()
    {
        // Default key config which takes place if key mappings could not be loaded from config file.

        // See InputModuleOIS for explanation
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_W,       Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_S,       Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_A,       Events::MOVE_LEFT_PRESSED,      Events::MOVE_LEFT_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_D,       Events::MOVE_RIGHT_PRESSED,     Events::MOVE_RIGHT_RELEASED,   0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_UP,      Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_DOWN,    Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_SPACE,   Events::MOVE_UP_PRESSED,        Events::MOVE_UP_RELEASED,      0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_C,       Events::MOVE_DOWN_PRESSED,      Events::MOVE_DOWN_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_PGUP,    Events::MOVE_UP_PRESSED,        Events::MOVE_UP_RELEASED,      0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_PGDOWN,  Events::MOVE_DOWN_PRESSED,      Events::MOVE_DOWN_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_LEFT,    Events::MOVE_LEFT_PRESSED,      Events::MOVE_LEFT_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_RIGHT,   Events::MOVE_RIGHT_PRESSED,     Events::MOVE_RIGHT_RELEASED,   0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FirstPerson, OIS::KC_F,       Events::TOGGLE_FLYMODE,         Events::TOGGLE_FLYMODE_REL,    0);

        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_W,       Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_S,       Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_A,       Events::ROTATE_LEFT_PRESSED,    Events::ROTATE_LEFT_RELEASED,  0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_D,       Events::ROTATE_RIGHT_PRESSED,   Events::ROTATE_RIGHT_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_UP,      Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_DOWN,    Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_SPACE,   Events::MOVE_UP_PRESSED,        Events::MOVE_UP_RELEASED,      0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_C,       Events::MOVE_DOWN_PRESSED,      Events::MOVE_DOWN_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_PGUP,    Events::MOVE_UP_PRESSED,        Events::MOVE_UP_RELEASED,      0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_PGDOWN,  Events::MOVE_DOWN_PRESSED,      Events::MOVE_DOWN_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_LEFT,    Events::ROTATE_LEFT_PRESSED,    Events::ROTATE_LEFT_RELEASED,  0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_RIGHT,   Events::ROTATE_RIGHT_PRESSED,   Events::ROTATE_RIGHT_RELEASED, 0);
        module_->RegisterUnbufferedKeyEvent(Input::State_ThirdPerson, OIS::KC_F,       Events::TOGGLE_FLYMODE,         Events::TOGGLE_FLYMODE_REL, 0);

        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_W,       Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED,  0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_S,       Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,     0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_A,       Events::MOVE_LEFT_PRESSED,      Events::MOVE_LEFT_RELEASED,     0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_D,       Events::MOVE_RIGHT_PRESSED,     Events::MOVE_RIGHT_RELEASED,    0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_UP,      Events::MOVE_FORWARD_PRESSED,   Events::MOVE_FORWARD_RELEASED,  0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_DOWN,    Events::MOVE_BACK_PRESSED,      Events::MOVE_BACK_RELEASED,     0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_SPACE,   Events::MOVE_UP_PRESSED,        Events::MOVE_UP_RELEASED,       0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_C,       Events::MOVE_DOWN_PRESSED,      Events::MOVE_DOWN_RELEASED,     0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_PGUP,    Events::MOVE_UP_PRESSED,        Events::MOVE_UP_RELEASED,       0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_PGDOWN,  Events::MOVE_DOWN_PRESSED,      Events::MOVE_DOWN_RELEASED,     0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_LEFT,    Events::ROTATE_LEFT_PRESSED,    Events::ROTATE_LEFT_RELEASED,   0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_RIGHT,   Events::ROTATE_RIGHT_PRESSED,   Events::ROTATE_RIGHT_RELEASED,  0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_J,       Events::ROTATE_LEFT_PRESSED,    Events::ROTATE_LEFT_RELEASED,   0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_L,       Events::ROTATE_RIGHT_PRESSED,   Events::ROTATE_RIGHT_RELEASED,  0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_I,       Events::ROTATE_UP_PRESSED,      Events::ROTATE_UP_RELEASED,     0);
        module_->RegisterUnbufferedKeyEvent(Input::State_FreeCamera, OIS::KC_K,       Events::ROTATE_DOWN_PRESSED,    Events::ROTATE_DOWN_RELEASED,   0);

        module_->RegisterUnbufferedKeyEvent(Input::State_All, OIS::KC_GRAVE,   Events::SHOW_DEBUG_CONSOLE,     Events::SHOW_DEBUG_CONSOLE_REL,  0);
        module_->RegisterUnbufferedKeyEvent(Input::State_All, OIS::KC_TAB,     Events::SWITCH_CAMERA_STATE,    Events::SWITCH_CAMERA_STATE_REL, 0);

        module_->RegisterSliderEvent(Input::State_FirstPerson, SliderMouse,    Events::MOUSELOOK, Events::MOUSELOOK_STOPPED, OIS::MB_Right);
        module_->RegisterSliderEvent(Input::State_ThirdPerson, SliderMouse,    Events::MOUSELOOK, Events::MOUSELOOK_STOPPED, OIS::MB_Right);
        module_->RegisterSliderEvent(Input::State_FreeCamera,  SliderMouse,    Events::MOUSELOOK, Events::MOUSELOOK_STOPPED, OIS::MB_Right);

        module_->RegisterMouseButtonEvent(Input::State_All, OIS::MB_Left, Events::INWORLD_CLICK, Events::INWORLD_CLICK_REL, Input::All);
        module_->RegisterMouseButtonEvent(Input::State_All, OIS::MB_Left, Events::INWORLD_CLICK_BUILD, Events::INWORLD_CLICK_BUILD_REL, OIS::Keyboard::Ctrl);
        module_->RegisterMouseButtonEvent(Input::State_All, OIS::MB_Right, Events::RIGHT_MOUSECLICK_PRESSED, Events::RIGHT_MOUSECLICK_RELEASED,0);
    }
}

