// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputModuleOIS_h
#define incl_InputModuleOIS_h

#include "ModuleInterface.h"

namespace Foundation
{
   class Framework;
}

// Input related functionality. May be keyboard, mouse, game controllers or anything
namespace Input
{
    //! Input module that uses OIS for input
    class InputModuleOIS : public Foundation::ModuleInterfaceImpl
    {
    public:
        InputModuleOIS();
        virtual ~InputModuleOIS();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update(Core::f64 frametime);

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Input;


    private:
    };
}
#endif
