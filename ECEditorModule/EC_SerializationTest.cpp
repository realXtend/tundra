// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "EC_SerializationTest.h"
#include "ModuleInterface.h"
#include "XMLUtilities.h"

#include <QDomDocument>

using namespace RexTypes;

namespace ECEditor
{
    EC_SerializationTest::EC_SerializationTest(Foundation::ModuleInterface* module) :
        Foundation::ComponentInterface(module->GetFramework())
    {
    }
    
    EC_SerializationTest::~EC_SerializationTest()
    {
    }
    
    void EC_SerializationTest::SerializeTo(QDomDocument& doc, QDomElement& base_element) const
    {
        QDomElement comp_element = BeginSerialization(doc, base_element);
        WriteAttribute(doc, comp_element, "vector", WriteVector3(attr1_));
        WriteAttribute(doc, comp_element, "quat", WriteQuaternion(attr2_));
        WriteAttribute(doc, comp_element, "string", attr3_);
    }
    
    void EC_SerializationTest::DeserializeFrom(QDomElement& element)
    {
        // Check that type is right, otherwise do nothing
        if (!BeginDeserialization(element))
            return;
        
        attr1_ = ParseVector3(ReadAttribute(element, "vector"));
        attr2_ = ParseQuaternion(ReadAttribute(element, "quat"));
        attr3_ = ReadAttribute(element, "string");
        
        attr2_.normalize();
        
        OnChanged();
    }
}

