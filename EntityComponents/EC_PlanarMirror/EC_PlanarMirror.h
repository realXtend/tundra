/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_PlanarMirror.h
 *  @brief  EC_PlanarMirror enables one to create planar mirrors.
 *  @note   The entity should have EC_Placeable available in advance.
 */

#ifndef incl_EC_PlanarMirror_EC_PlanarMirror_h
#define incl_EC_PlanarMirror_EC_PlanarMirror_h
#include "StableHeaders.h"
#include "IComponent.h"
#include "Declare_EC.h"
#include "Renderer.h"
#include <QVector3D>
#include <QQuaternion>


namespace Ogre
{
    class Camera;
    class Texture;
    class Entity;
    class MovablePlane;
}

/**
<table class="header">
<tr>
<td>
<h2>PlanarMirror</h2>
EntityComponent that will create a planar mirror texture (and optionally a plane showing it).
NOTE: Assumes the the entity already has: EC_Placeable, EC_RttTarget and EC_OgreCamera. Otherwise EC_PlanarMirror cannot function.
<b>Attributes</b>:
<ul>
<li>bool reflectionPlaneVisible
<div>do we want to show the mirror plane</div>
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
class EC_PlanarMirror : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_PlanarMirror);
    

public:

    EC_PlanarMirror(IModule *module);
    ~EC_PlanarMirror();

    Q_PROPERTY(bool reflectionPlaneVisible READ getreflectionPlaneVisible WRITE setreflectionPlaneVisible);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, reflectionPlaneVisible);


    //Returns the texture that you can set to be used on a material. Do not modify this texture yourself
    Ogre::Texture* GetMirrorTexture();
public slots:
    void Initialize();
    void Update(float val);
    void AttributeUpdated(IAttribute* attr);


private:
    void CreatePlane();
    static int mirror_cam_num_;
    boost::weak_ptr<OgreRenderer::Renderer> renderer_;
    Ogre::Camera* mirror_cam_;
    Ogre::Texture* mirror_texture_;
    Ogre::TextureUnitState* tex_unit_state_;
    Ogre::Material* mat_;
    Ogre::Entity* mirror_plane_entity_;
    Ogre::MovablePlane* mirror_plane_;
};

#endif
