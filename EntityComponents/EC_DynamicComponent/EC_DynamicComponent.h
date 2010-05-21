// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_DynamicComponent_EC_DynamicComponent_h
#define incl_EC_DynamicComponent_EC_DynamicComponent_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"

class QDomDocument;
class QDomElement;

#include <qvariant.h>

//! A 
/*! 
 */
class EC_DynamicComponent : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_DynamicComponent);

    Q_OBJECT

public:
    //! Destructor.
    ~EC_DynamicComponent();

    virtual bool IsSerializable() const { return true; }

    //! Position within source (for example scrollbar value, page number)
    Foundation::Attribute<int> position_;

public slots:
    void AddAttribute();
    QVariant GetAttribute();
    //private slots:
    
private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_DynamicComponent(Foundation::ModuleInterface *module);

};

#endif
