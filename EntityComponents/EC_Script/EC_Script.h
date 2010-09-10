// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Script_EC_Script_h
#define incl_EC_Script_EC_Script_h

#include "ComponentInterface.h"
//#include "AttributeInterface.h"
#include "Declare_EC.h"
#include "CoreTypes.h"

using Foundation::AttributeInterface;

class EC_Script: public Foundation::ComponentInterface
{
    Q_OBJECT
    DECLARE_EC(EC_Script)
public:
    ~EC_Script();

    virtual bool IsSerializable() const { return true; }

    Foundation::Attribute<QString> scriptRef_;

signals:
    void onScriptRefChanged(const QString &newValue);

private slots:
    void HandleAttributeChanged(AttributeInterface* attribute, AttributeChange::Type change);

private:
    explicit EC_Script(Foundation::ModuleInterface *module);
};

#endif