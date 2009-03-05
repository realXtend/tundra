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

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);

        virtual void Update();

        virtual bool Test() const { return true; }

    private:
        Foundation::Framework *framework_;
    };
}
#endif
