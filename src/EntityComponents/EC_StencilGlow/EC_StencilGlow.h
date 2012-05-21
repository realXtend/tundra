/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_StencilGlow.h
 *  @brief  Adds an outline to a mesh.
 */

#pragma once

#include "IComponent.h"
#include "EC_Mesh.h"
#include "OgreWorld.h"

#include <Ogre.h>
#include <OgreRenderQueueListener.h>

// render queue groups
#define STENCIL_GLOW_ENTITY Ogre::RENDER_QUEUE_MAIN + 1
#define STENCIL_GLOW_OUTLINE Ogre::RENDER_QUEUE_OVERLAY - 1

class EC_StencilGlow : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_StencilGlow", 108)
public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_StencilGlow(Scene* scene);
    ~EC_StencilGlow();

    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

private slots:
    /// Called upon parent entity set
    void Initialize();

    /// Called when the mesh is changed so that the stencil be redrawn
    void OnMeshChanged();

    /// Called when the mesh is destroyed, so that the stencil be destroyed first
    void OnMeshAboutToBeDestroyed();

    /// Called when some of the attributes has been changed.
    void OnAttributeUpdated(IAttribute *attribute);

private:

    void CreateStencilGlow();

    void DestroyStencilGlow();

    void SetStencilGlowEnabled(bool);

    EC_Mesh* GetMesh() const;

    Ogre::Entity* outlineEntity_;
    Ogre::SceneNode* outlineSceneNode_;

    OgreWorldWeakPtr world_;
};
