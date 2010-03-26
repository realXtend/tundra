// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputMapper_h
#define incl_InputMapper_h

#include "InputModuleOIS.h"
#include "InputServiceInterface.h"

namespace Input
{
    namespace Events
    {
        class Movement;
    }

    //! Maps hardware input codes into more generic input events.
    /*! Default map is used always at first, but if a mappings xml file exists,
        the input mappings are read from that file and they replace the default mappings.
        If a mapping for some event is not found from the xml file, the default one is
        used instead.

        \todo We constantly access InputModuleOIS from here for many service functions, 
              perhaps needs rethinking, maybe even turn InputModuleOIS itself into input service. -cm
    */
    class Mapper : public Input::InputServiceInterface
    {
        //! empty default constructor
        Mapper();
    public:
        //! constructor that takes parent module
        Mapper(InputModuleOIS *module);

        //! destructor
        ~Mapper();

        bool Poll(event_id_t input_event) const { return module_->IsEvent(input_event); }

        boost::optional<const Input::Events::Movement&> PollSlider(event_id_t dragged_event) const { return module_->GetDraggedSliderInfo(dragged_event); }

        //! Loads input mappings from xml file.
        /*! Replaces the default mappings with ones loaded from the file. In case of an error
            all or some of the default mappings may not get replaced.

            \param file full path to the xml file
        */
        void LoadInputMappings(const std::string &file);

        //! Loads input mappings from the specified xml node. See LoadInputMappings(const std::string &file) for more information.
        /*!
            \param node xml (root) node that contains input mappings
        */
        void LoadInputMappings(const Poco::XML::Node* node);

        //! Exports mappings to a file
        void Export(const std::string &file);

        void SetState(State state) { module_->SetState(state); }
        State GetState() const { return module_->GetState(); }

    private:

        //! Sets default mappings, if no mappings file exists
        void SetDefaultMappings();

        //! parent module
        InputModuleOIS *module_;
    };
}

#endif
