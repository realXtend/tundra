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
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>();
    
    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(Update3DGizmo()));

}

EC_3DGizmo::~EC_3DGizmo()
{
}

void EC_3DGizmo::SetEditableAttributes(QList<IAttribute*> attributes)
{
	attributes_ = attributes;
	
	for( int i = 0; i < attributes_.size(); i++)
	{
		IAttribute *attribute = attributes.at(i);
		std::cout << attribute->GetNameString() << std::endl;
	}
}

void EC_3DGizmo::Update3DGizmo()
{
    std::cout << "3dgizmo update" << std::endl;
}
