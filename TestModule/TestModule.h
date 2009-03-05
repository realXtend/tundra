// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TestModule_h
#define incl_TestModule_h

#include "ModuleInterface.h"
#include "TestServiceInterface.h"

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
    class TestModule : public Foundation::ModuleInterface_Impl, public Foundation::TestServiceInterface
    {
    public:
        TestModule();
        virtual ~TestModule();

        virtual void load();
        virtual void unload();
        virtual void initialize(Foundation::Framework *framework);
        virtual void uninitialize(Foundation::Framework *framework);

        virtual void update();

        virtual bool test() const { return true; }

    private:
        Foundation::Framework *mFramework;
    };
}
#endif
