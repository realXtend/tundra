// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleApi.h"
#include "IComponent.h"
#include "OgreWorld.h"

#include "Math/float4.h"

#include <OgreGpuProgramParams.h>

/// Provides a way to configure client side shadow setup during runtime per scene.
class OGRE_MODULE_API EC_OgreShadowSetup : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("OgreShadowSetup", 32)

public:
    /// @cond PRIVATE
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_OgreShadowSetup(Scene* scene);
    /// @endcond
    virtual ~EC_OgreShadowSetup();

    /// Shadow split lamba. Default is 0.93.
    /** Lower lamdba means more uniform, higher lambda means more logarithmic. */
    Q_PROPERTY(float splitLambda READ getsplitLambda WRITE setsplitLambda);
    DEFINE_QPROPERTY_ATTRIBUTE(float, splitLambda);

    /// First split distance. Default is 8.5.
    Q_PROPERTY(float firstSplitDist READ getfirstSplitDist WRITE setfirstSplitDist);
    DEFINE_QPROPERTY_ATTRIBUTE(float, firstSplitDist);

    /// Shadow far distance. Default is 500.0f.
    Q_PROPERTY(float farDist READ getfarDist WRITE setfarDist);
    DEFINE_QPROPERTY_ATTRIBUTE(float, farDist);

    /// Shadow fade distance. Default is 300.0.
    Q_PROPERTY(float fadeDist READ getfadeDist WRITE setfadeDist);
    DEFINE_QPROPERTY_ATTRIBUTE(float, fadeDist);

    /// 1st shadow depth bias
    /** Default for all depth biases is 0.05.
        @note There are four depth bias values. float attribute type were used 
        instead of float4 so people wont get confused by the axis labeling (eg. in UI). */
    Q_PROPERTY(float depthBias1 READ getdepthBias1 WRITE setdepthBias1);
    DEFINE_QPROPERTY_ATTRIBUTE(float, depthBias1);

    /// 2nd shadow depth bias
    Q_PROPERTY(float depthBias2 READ getdepthBias2 WRITE setdepthBias2);
    DEFINE_QPROPERTY_ATTRIBUTE(float, depthBias2);

    /// 3rd shadow depth bias
    Q_PROPERTY(float depthBias3 READ getdepthBias3 WRITE setdepthBias3);
    DEFINE_QPROPERTY_ATTRIBUTE(float, depthBias3);

    /// 4th shadow depth bias
    Q_PROPERTY(float depthBias4 READ getdepthBias4 WRITE setdepthBias4);
    DEFINE_QPROPERTY_ATTRIBUTE(float, depthBias4);

private slots:
    void OnParentEntitySet();

private:
    /// Get Ogre scenemanager
    Ogre::SceneManager* OgreSceneManager() const;

    /// IComponent override.
    void AttributesChanged();

    /// Update shadow setup
    void UpdateShadowSetup();

    /// Shared shadow parameters
    Ogre::GpuSharedParametersPtr shadowParams_;

    /// Ogre world
    OgreWorldWeakPtr world_;
};
