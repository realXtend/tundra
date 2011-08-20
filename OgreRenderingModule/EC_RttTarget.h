// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_EC_RttTarget_h
#define incl_OgreRenderer_EC_RttTarget_h

#include "StableHeaders.h"
#include "OgreModuleApi.h"
#include "IComponent.h"
#include "Declare_EC.h"
#include "Core.h"


namespace OgreRenderer { class OgreRenderingModule; };

/// Ogre render-to-texture component
/**
<table class="header">
<tr>
<td>
<h2>RttTarget</h2>
Render-to-texture target: renders a view from a camera to a texture.
Registered by OgreRenderer::OgreRenderingModule.

<b>Attributes</b>:
<ul>
<li>QString targettexture
<div>Name of the target texture where to render the image
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

<b>Depends on a camera component.</b>.
</table>
*/
class OGRE_MODULE_API EC_RttTarget : public IComponent
{
    Q_OBJECT
    
    DECLARE_EC(EC_RttTarget);

public:
    Q_PROPERTY(QString targettexture READ gettargettexture WRITE settargettexture);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, targettexture);

    Q_PROPERTY(int size_x READ getsize_x WRITE setsize_x);
    DEFINE_QPROPERTY_ATTRIBUTE(int, size_x);

    Q_PROPERTY(int size_y READ getsize_y WRITE setsize_y);
    DEFINE_QPROPERTY_ATTRIBUTE(int, size_y);

    virtual ~EC_RttTarget();

    //! Set component as serializable.
    virtual bool IsSerializable() const { return true; }

public slots:
    void PrepareRtt();
    void SetAutoUpdated(bool val);

    /// Get Texture in raw format
    Ogre::uchar* GetRawTexture(int texture_width, int texture_height);        

private slots:
    void OnAttributeUpdated(IAttribute* attribute);
    //void UpdateRtt();

private:
    //! constructor
    /*! \param module Ogre module
     */
    EC_RttTarget(IModule* module);

    //! Owner module of this component
    //OgreRenderer::OgreRenderingModule *owner_;

    Ogre::TexturePtr tex_;
    std::string material_name_;
    //void ScheduleRender();

    Ogre::uchar *pixelData_;
};

#endif
