/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_3DGizmo.h
 *  @brief  EC_3DGizmo enables manipulators for values
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_3DGizmo.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_OpenSimPrim.h"
#include "LoggingFunctions.h"
#include "RexUUID.h"
#include <Ogre.h>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_3DGizmo")

#include "MemoryLeakCheck.h"

EC_3DGizmo::EC_3DGizmo(IModule *module) :
    IComponent(module->GetFramework())
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);

	_type = EC_3DGizmo::NotSet;
    
    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(Update3DGizmo()));

}

EC_3DGizmo::~EC_3DGizmo()
{
}

void EC_3DGizmo::SetGizmo(QString gizmo)
{
}

void EC_3DGizmo::StartDrag()
{
	emit dragStarted();
}

void EC_3DGizmo::EndDrag()
{
	emit dragCompleted();
}

void EC_3DGizmo::Drag(int x, int y, QVector3D &changevec)
{
}

void EC_3DGizmo::SetDatum(QVector3D &vec3)
{
	_type = EC_3DGizmo::Vector3;
	_vec3 = vec3;
}

void EC_3DGizmo::SetDatum(QQuaternion &quat)
{
	_type = EC_3DGizmo::Quaternion;
	_quat = quat;
}

void EC_3DGizmo::SetDatum(QVector2D &vec2)
{
	_type = EC_3DGizmo::Vector2;
	_vec2 = vec2;
}

void EC_3DGizmo::SetDatum(float scalar)
{
	_type = EC_3DGizmo::Scalar;
	_scalar = scalar;
}

void EC_3DGizmo::Update3DGizmo()
{
    std::cout << "3dgizmo update" << std::endl;
}
