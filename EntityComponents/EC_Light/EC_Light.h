// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Light_EC_Light_h
#define incl_EC_Light_EC_Light_h

#include "ComponentInterface.h"
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
    virtual void SerializeTo(QDomDocument& doc, QDomElement& base_element) const;
    virtual void DeserializeFrom(QDomElement& element, Foundation::ChangeType change);

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
    Vector3df direction_;
    
    //! Light type
    Type type_;
    
    //! Light diffuse color
    Color diffuse_color_;
    
    //! Light specular color
    Color specular_color_;
    
    //! Cast shadows flag
    bool cast_shadows_;
    
    //! Light range
    Real range_;
    
    //! Light constant attenuation
    Real constant_atten_;
    
    //! Light linear attenuation
    Real linear_atten_;
    
    //! Light quadratic attenuation
    Real quadratic_atten_;
    
    //! Spotlight inner angle (degrees)
    Real inner_angle_;
    
    //! Spotlight outer angle (degrees)
    Real outer_angle_;
    
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
