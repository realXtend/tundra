// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestModuleB_h
#define incl_TestModuleB_h

#include "ModuleInterface.h"
#include "TestServiceInterface.h"

using namespace std;

namespace Foundation
{
   class Framework;
}

namespace Test
{
    class TestModuleB : public Foundation::ModuleInterfaceImpl
    {
    public:
        TestModuleB();
        virtual ~TestModuleB();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void Uninitialize();
        virtual void PostInitialize();

        virtual void Update(Core::f64 frametime);

        virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);

        MODULE_LOGGING_FUNCTIONS

        static const std::string &NameStatic()
        {
            static const std::string name("TestModuleB");
            return name;
        }

    private:
    };
}
#endif
