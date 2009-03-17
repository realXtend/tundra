// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleModule_h
#define incl_ConsoleModule_h

#include "ModuleInterface.h"
#include "Native.h"

namespace Foundation
{
    class Framework;
}

namespace Console
{
    //! interface for modules
    class REX_API ConsoleModule : public Foundation::ModuleInterface_Impl
    {
    public:
        ConsoleModule();
        virtual ~ConsoleModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);

        virtual void Update();



        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Console;

    private:
        //! native debug console
        Native native_;

        Foundation::Framework *framework_;
    };
}

#endif
