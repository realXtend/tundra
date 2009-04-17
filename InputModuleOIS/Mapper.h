// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputMapper_h
#define incl_InputMapper_h

#include "InputModuleOIS.h"

namespace Input
{
    namespace Events
    {
        class Movement;
    }

    //! Maps hardware input codes into more generic input events
    /*!
        If default mappings file doesn't already exist, exports one. This is
        important because key strings are locale specific, f.ex. 'Enter' might
        be something else in other languages.
    */
    class Mapper
    {
        //! default constructor
        Mapper();
    public:
        //! constructor that takes parent module
        Mapper(InputModuleOIS *module);

        //! destructor
        ~Mapper();

        //! Polls the current mouse state for both absolute and relative movement
        /*! Not thread safe
        */
        __inline const Events::Movement &GetMouseMovement() const { return module_->GetMouseMovement(); }

        //! Loads input mappings from xml file, or uses default ones if the file is not found
        /*!
            \param file full path to the xml file
        */
        void LoadInputMappings(const std::string &file);

        //! Loads input mappings from the specified xml node. See LoadInputMappings(const std::string &file) for more information.
        /*!
            \param node xml (root) node that contains input mappings
        */
        void LoadInputMappings(const Poco::XML::Node* node);

    private:
        //! Exports default mappings to a file
        void ExportDefaults();

        //! Sets default mappings, if no mappings file exists
        void SetDefaultMappings();

        //! parent module
        InputModuleOIS *module_;
    };
}

#endif
