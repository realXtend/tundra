/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SkyX.h
 *  @brief  A sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
 */

#pragma once

#include "IComponent.h"

struct EC_SkyXImpl;

/// A Sky component using SkyX, http://www.ogre3d.org/tikiwiki/SkyX
/** @note Requires SkyX Ogre add-on. */
class EC_SkyX : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_SkyX", 666)

public:
    /// Do not directly allocate new components using operator new, but use the factory-based SceneAPI::CreateComponent functions instead.
    explicit EC_SkyX(Scene* scene);
    ~EC_SkyX();

    /// 
//    DEFINE_QPROPERTY_ATTRIBUTE(float, timeMultiplier);
//    Q_PROPERTY(float timeMultiplier READ gettimeMultiplier WRITE settimeMultiplier);

private:
    EC_SkyXImpl *impl;

private slots:
    void Update(float frameTime);
};
