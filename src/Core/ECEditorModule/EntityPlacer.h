// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "InputFwd.h"
#include "SceneFwd.h"
#include "OgreModuleFwd.h"
#include "Math/float3.h"
#include "Math/Quat.h"
#include "CoreTypes.h"

#include <QObject>

class Framework;
class EC_Placeable;

/// @todo Is this needed anymore? If not, delete.
/// @cond PRIVATE
class EntityPlacer : public QObject
{
    Q_OBJECT

public:
    /// If entity that we want to place in our world wont hold any mesh component. We use default mesh to
    /// tell the user where hes placing his mesh (e.g. placing sound source on the scene).
    //static const std::string CustomMeshName;

    EntityPlacer(Framework *framework, entity_id_t entityId = 0, QObject *parent = 0);
    virtual ~EntityPlacer();

public slots:
    void OnMouseEvent(MouseEvent *mouse);
    void MouseMove(MouseEvent *mouse);

signals:
    void Finished(float3 location, Quat orientation);
    void LocationChanged(float3 location);
    void OrientationChanged(Quat orientation);

private:
     bool DoRayCast(int x, int y, float3 &result);

private:
    float3 location_;
    Quat orientation_;
    InputContextPtr input_;
    EC_Placeable *placeable_;
    Framework *framework_;
    EntityWeakPtr entity_;
    Ogre::Entity *meshEntity_;
    OgreWorldWeakPtr world_;
    int previousScrollValue_;
    bool finished_;
    bool useCustomMesh_;
};
/// @endcond
