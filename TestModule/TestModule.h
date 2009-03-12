// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestModule_h
#define incl_TestModule_h

#include "ModuleInterface.h"
#include "TestServiceInterface.h"
#include "TestService.h"

namespace Foundation
{
   class Framework;
}

//! Contains unit tests
/*! All Core and Foundation classes should be unit tested.
*/
namespace Test
{
    //! interface for modules
    class TestModule : public Foundation::ModuleInterface_Impl
    {
    public:
        TestModule();
        virtual ~TestModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);

        virtual void Update();

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::Type_Test;


    private:
        Foundation::Framework *framework_;
        TestService test_service_;
    };
}
#endif
