// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestModule_h
#define incl_TestModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "TestServiceInterface.h"
#include "TestService.h"
#include "EventDataInterface.h"
#include "ComponentInterface.h"



namespace Foundation
{
   class Framework;
}

//! Contains unit tests
/*! All Core and Foundation classes should be unit tested.
*/
namespace Test
{
    class TestEvent : public Foundation::EventDataInterface
    {
    public:
        TestEvent() : Foundation::EventDataInterface() {}
        virtual ~TestEvent() {}

        int test_value_;
    };

    //! interface for modules
    class TestModule : public Foundation::ModuleInterfaceImpl
    {
    public:
        TestModule();
        virtual ~TestModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();

        virtual void Update(f64 frametime);

        MODULE_LOGGING_FUNCTIONS

        //! returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

        static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Test;

    private:
        TestServicePtr test_service_;
    };
}
#endif
