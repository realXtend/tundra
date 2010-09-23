// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_SerializationTest.h"
#include "IModule.h"

namespace ECEditor
{
    EC_SerializationTest::EC_SerializationTest(IModule* module) :
        IComponent(module->GetFramework()),
        attr1_(this, "attr1"),
        attr2_(this, "attr2"),
        attr3_(this, "attr3"),
        attr4_(this, "attr4")
    {
    }
    
    EC_SerializationTest::~EC_SerializationTest()
    {
    }
}

