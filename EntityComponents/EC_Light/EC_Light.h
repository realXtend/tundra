// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Light_EC_Light_h
#define incl_EC_Light_EC_Light_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "Declare_EC.h"
#include "Vector3D.h"
#include "Color.h"

class QDomDocument;
class QDomElement;

namespace Ogre
{
    class Light;
}


class EC_Light : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Light);
    
    Q_OBJECT
    
public:
    //! light type enumeration
    enum Type
    {
        LT_Point,
        LT_Spot,
        LT_Directional
    };
    
    //! Destructor.
    ~EC_Light();
    
    virtual bool IsSerializable() const { return true; }

    //! Gets placeable component
    Foundation::ComponentPtr GetPlaceable() const { return placeable_; }
    
    //! Sets placeable component
    /*! Set a null placeable (or do not set a placeable) to have a detached light
        \param placeable placeable component
     */
    void SetPlaceable(Foundation::ComponentPtr placeable);
    
    //! @return Ogre light pointer
    Ogre::Light* GetLight() const { return light_; }
    
    //! Light direction
    Foundation::Attribute<Vector3df> directionAttr_;
    
    //! Light type
    Foundation::Attribute<int> typeAttr_;
    
    //! Light diffuse color
    Foundation::Attribute<Color> diffColorAttr_;
    
    //! Light specular color
    Foundation::Attribute<Color> specColorAttr_;
    
    //! Cast shadows flag
    Foundation::Attribute<bool> castShadowsAttr_;
    
    //! Light range
    Foundation::Attribute<Real> rangeAttr_;
    
    //! Light constant attenuation
    Foundation::Attribute<Real> constAttenAttr_;
    
    //! Light linear attenuation
    Foundation::Attribute<Real> linearAttenAttr_;
    
    //! Light quadratic attenuation
    Foundation::Attribute<Real> quadraAttenAttr_;
    
    //! Spotlight inner angle (degrees)
    Foundation::Attribute<Real> innerAngleAttr_;
    
    //! Spotlight outer angle (degrees)
    Foundation::Attribute<Real> outerAngleAttr_;
    

private slots:
    void UpdateOgreLight();
    
private:
    //! Constuctor.
    /*! \param module Module.
     */
    explicit EC_Light(Foundation::ModuleInterface *module);
    
    //! Attaches light to placeable
    void AttachLight();
    
    //! Detaches light from placeable
    void DetachLight();
    
    //! Placeable component, optional
    Foundation::ComponentPtr placeable_;
    
    //! Ogre light
    Ogre::Light* light_;
    
    //! Attached flag
    bool attached_;


};

#endif
