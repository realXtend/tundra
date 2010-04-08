// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_EC_SerializationTest_h
#define incl_ECEditorModule_EC_SerializationTest_h

#include "Framework.h"
#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Quaternion.h"

namespace ECEditor
{
    class EC_SerializationTest : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_SerializationTest);

        Q_OBJECT

    public:
        virtual ~EC_SerializationTest();

        virtual void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;
        virtual void DeserializeFrom(QDomElement& element);
        
        Vector3df attr1_;
        Quaternion attr2_;
        std::string attr3_;
        
    private:
        EC_SerializationTest(Foundation::ModuleInterface* module);
    };
}

#endif
