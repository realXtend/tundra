/**
    For conditions of distribution and use, see copyright notice in license.txt

    @file   EC_StencilGlow.h
    @brief  Adds an outline to a mesh. */

#pragma once

#include "IComponent.h"
#include "OgreModuleFwd.h"
#include "Color.h"
#include "Math/float3.h"

/// Adds an outline to a mesh
class EC_StencilGlow : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("StencilGlow", 108)

public:
    /// @cond PRIVATE
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_StencilGlow(Scene* scene);
    /// @endcond
    ~EC_StencilGlow();

    /// Defines the whether the outline is visible, true by default.
    Q_PROPERTY(bool enabled READ getenabled WRITE setenabled)
    DEFINE_QPROPERTY_ATTRIBUTE(bool, enabled);

    /// Defines the color of the outline, (1, 1, 1, 0.4) by default.
    Q_PROPERTY(Color color READ getcolor WRITE setcolor)
    DEFINE_QPROPERTY_ATTRIBUTE(Color, color);

    /// Defines the scale of the outline, (1.2, 1.2, 1.2) by default.
    Q_PROPERTY(float3 scale READ getscale WRITE setscale)
    DEFINE_QPROPERTY_ATTRIBUTE(float3, scale);

private slots:
    /// Called upon parent entity set
    void Initialize(); 

    /// Called when the mesh is changed so that the stencil be redrawn
    void OnMeshChanged();

    /// Called when the mesh is destroyed, so that the stencil be destroyed first
    void OnMeshAboutToBeDestroyed();

private:
    void AttributesChanged();

    void CreateStencilGlow();

    void DestroyStencilGlow();

    void SetStencilGlowEnabled(bool);

    EC_Mesh* GetMesh() const;

    Ogre::Entity* outlineEntity_;
    Ogre::SceneNode* outlineSceneNode_;

    OgreWorldWeakPtr world_;

    bool isEnabled;
};
