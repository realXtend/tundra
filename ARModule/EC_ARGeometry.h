
#ifndef incl_EC_ARGeometry_EC_ARGeometry_h
#define incl_EC_ARGeometry_EC_ARGeometry_h

#include "StableHeaders.h"
#include "ARFwd.h"
#include "OgreModuleFwd.h"

#include "IComponent.h"
#include "Declare_EC.h"

/**
<table class="header"><tr><td>
<h2>EC_ARGeometry</h2>

This is an EC to be attached to entities. The only purpose of this EC is to notify the ARRenderer that this object is to be rendered to the color buffer depending on the ARVisible attribute.

</td></tr></table>
*/
class EC_ARGeometry : public IComponent
{

DECLARE_EC(EC_ARGeometry);
Q_OBJECT   

public:
    ~EC_ARGeometry();

    /// Do we want to show the mirror plane.
    /// \todo be more precise on above
    Q_PROPERTY(bool ARVisible READ getARVisible WRITE setARVisible);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, ARVisible);

    virtual bool IsSerializable() const { return true; }

public slots:
    Ogre::Entity* GetOgreEntity();

private slots:
    void ParentReady();
    void MeshReady();

private:
    explicit EC_ARGeometry(IModule *module);
    Ogre::Entity* ogre_entity_;
};

#endif
