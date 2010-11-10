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
    
    attributes_ = QList<IAttribute *>();

}

EC_3DGizmo::~EC_3DGizmo()
{
}

void EC_3DGizmo::AddEditableAttribute(IComponent* component, QString &attribute_name)
{
	IAttribute* attribute = component->GetAttribute(attribute_name);
	attributes_ << attribute;
	
	for( int i = 0; i < attributes_.size(); i++)
	{
		IAttribute *attr = attributes_.at(i);
		std::cout << attr->GetNameString() << std::endl;
	}
}

void EC_3DGizmo::ClearEditableAttributes()
{
	attributes_.clear();
}

void EC_3DGizmo::Manipulate()
{
    std::cout << "3dgizmo update" << std::endl;
    for(int i = 0; i < attributes_.size(); i++) {
		IAttribute * attr = attributes_.at(i);
		AttributeMetadata *meta = attr->GetMetadata();
		std::cout << attr->GetNameString() << std::endl;
		if(meta) {
			std::cout << "meta:" << meta->description.toStdString() << std::endl;
		}
	}
    std::cout << " ... ok" << std::endl;
}
