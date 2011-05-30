
#include "StableHeaders.h"
#include "EC_ARGeometry.h"

#include "Renderer.h"
#include "Entity.h"
#include "EC_Mesh.h"

#include <OgreEntity.h>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_ARGeometry")

EC_ARGeometry::EC_ARGeometry(IModule *module) :
    IComponent(module->GetFramework()),
    ARVisible(this, "Geometry visible", false),
    ogre_entity_(0)
{
    connect(this, SIGNAL(ParentEntitySet()), SLOT(ParentReady()));
}

EC_ARGeometry::~EC_ARGeometry()
{
}

Ogre::Entity* EC_ARGeometry::GetOgreEntity()
{
    return ogre_entity_;
}

void EC_ARGeometry::ParentReady()
{
    if (!GetParentEntity())
    {
        LogError("Could not initialize, parent entity is not set.");
        return;
    }

    boost::shared_ptr<EC_Mesh> mesh = GetParentEntity()->GetComponent<EC_Mesh>();
    if (!mesh.get())
    {
        LogError("Could not initialize, there is no EC_Mesh in the parent entity.");
        return;
    }

    connect(mesh.get(), SIGNAL(MeshChanged()), SLOT(MeshReady()));
    ogre_entity_ = mesh->GetEntity();
}

void EC_ARGeometry::MeshReady()
{
    boost::shared_ptr<EC_Mesh> mesh = GetParentEntity()->GetComponent<EC_Mesh>();
    if (!mesh.get())
    {
        LogError("Could not initialize, there is no EC_Mesh in the parent entity.");
        return;
    }
    ogre_entity_ = mesh->GetEntity();
}
