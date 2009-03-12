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
    class TestModuleB : public Foundation::ModuleInterface_Impl
    {
    public:
        TestModuleB();
        virtual ~TestModuleB();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);

        virtual void Update();

        MODULE_LOGGING_FUNCTIONS

        static const std::string &NameStatic()
        {
            static const std::string name("TestModuleB");
            return name;
        }

    private:
        Foundation::Framework *framework_;
    };
}
#endif
