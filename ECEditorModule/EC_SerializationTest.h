// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_EC_SerializationTest_h
#define incl_ECEditorModule_EC_SerializationTest_h

#include "Framework.h"
#include "AttributeInterface.h"
#include "AssetInterface.h"
#include "ComponentInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Quaternion.h"

namespace ECEditor
{
/**
<table class="header">
<tr>
<td>
<h2>SerialazationTest</h2>

Registered by ECEditor::ECEditorModule.

<b>Attributes</b>:
<ul>
<li>Vector3df: attr1_
<div></div> 
<li>Quaternion: attr2_
<div></div> 
<li>QString: attr3_
<div></div> 
<li>Foundation::AssetReference : attr4_
<div></div> 
</ul>


<b>Exposes the following scriptable functions:</b>
<ul>
<li>...
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>...
</ul>
</td>
</tr>

Does not emit any actions.

<b>Doesn't depend on other Entity Components</b>.
</table>

*/
    class EC_SerializationTest : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_SerializationTest);

        Q_OBJECT

    public:
        virtual ~EC_SerializationTest();

        virtual bool IsSerializable() const { return true; }
        
        Attribute<Vector3df> attr1_;
        Attribute<Quaternion> attr2_;
        Attribute<QString> attr3_;
        Attribute<Foundation::AssetReference> attr4_;
        
    private:
        EC_SerializationTest(Foundation::ModuleInterface* module);
    };
}

#endif
