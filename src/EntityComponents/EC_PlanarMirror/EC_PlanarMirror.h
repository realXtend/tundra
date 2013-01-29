/**
    For conditions of distribution and use, see copyright notice in LICENSE
 
    @file   EC_PlanarMirror.h
    @brief  Enables one to create planar mirrors. */

#pragma once

#include "IComponent.h"
#include "OgreModuleFwd.h"

#include <OgreTexture.h>

/// Creates a planar mirror texture (and optionally a plane showing it).
/** <table class="header">
    <tr>
    <td>
    <h2>PlanarMirror</h2>
    Creates a planar mirror texture (and optionally a plane showing it).

    <b>Attributes</b>:
    <ul>
    <li>bool reflectionPlaneVisible
    <div>@copydoc reflectionPlaneVisible</div>
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

    <b>Depends on components @ref EC_Camera "Camera", @ref EC_Placeable "Placeable" and @ref EC_RttTarget "RttTarget".</b>
    </table> */
class EC_PlanarMirror : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_PlanarMirror", 34)

public:
    EC_PlanarMirror(Scene *scene);
    ~EC_PlanarMirror();

    /// Do we want to show the mirror plane
    Q_PROPERTY(bool reflectionPlaneVisible READ getreflectionPlaneVisible WRITE setreflectionPlaneVisible);
    DEFINE_QPROPERTY_ATTRIBUTE(bool, reflectionPlaneVisible);

    //Returns the texture that you can set to be used on a material. Do not modify this texture yourself
    Ogre::Texture* GetMirrorTexture() const;

public slots:
    void Initialize();
    void Update(float val);
    void WindowResized(int w,int h);

private:
    void CreatePlane();
    void AttributesChanged();

    OgreRenderer::RendererPtr renderer_;
    
    static int mirror_cam_num_;
    
    Ogre::Camera* mirror_cam_;
    Ogre::Texture* mirror_texture_;
    Ogre::TextureUnitState* tex_unit_state_;
    Ogre::Material* mat_;
    Ogre::Entity* mirror_plane_entity_;
    Ogre::MovablePlane* mirror_plane_;
};
