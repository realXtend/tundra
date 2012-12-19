// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IComponent.h"
#include "OgreModuleApi.h"

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
<li>QString textureName
<div>Name of the target texture where to render the image.
<li>int width
<div>Width of the texture.
<li>int height
<div>Height of the texture.
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

<b>Depends on EC_Camera.</b>.
</table>
*/
class OGRE_MODULE_API EC_RttTarget : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_RttTarget", 21)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_RttTarget(Scene* scene);
    virtual ~EC_RttTarget();

    /// Name of the target texture where to render the image.
    Q_PROPERTY(QString textureName READ gettextureName WRITE settextureName);
    DEFINE_QPROPERTY_ATTRIBUTE(QString, textureName);

    /// Width of the texture.
    Q_PROPERTY(int width READ getwidth WRITE setwidth);
    DEFINE_QPROPERTY_ATTRIBUTE(int, width);

    /// Height of the texture.
    Q_PROPERTY(int height READ getheight WRITE setheight);
    DEFINE_QPROPERTY_ATTRIBUTE(int, height);

public slots:
    void PrepareRtt();
    void SetAutoUpdated(bool val);

//private slots:
    //void UpdateRtt();

private:

    void AttributesChanged();

    std::string material_name_;
    //void ScheduleRender();
};
