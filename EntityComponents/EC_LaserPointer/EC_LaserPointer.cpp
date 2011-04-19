/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_LaserPointer.cpp
 *  @brief  EC_LaserPointer adds laser pointer to entities.
 *  @note   The entity must have EC_Placeable and EC_InputMapper available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EC_LaserPointer.h"
#include "IModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "EC_InputMapper.h"
#include "Entity.h"
#include "OgreMaterialUtils.h"
#include "LoggingFunctions.h"
#include "SceneManager.h"

#include <QTimer>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_LaserPointer");

#include <Ogre.h>

EC_LaserPointer::EC_LaserPointer(IModule *module) :
    IComponent(module->GetFramework()),
    startPos_(this, "startPosition"),
    endPos_(this, "endPosition"),
    enabled_(this, "enabled", true),
    laserObject_(0),
    id_(),
    node_(0),
    input_(0),
    canUpdate_(true),
    updateInterval_(20)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer);
    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(CreateLaser()));
}

EC_LaserPointer::~EC_LaserPointer()
{
    disconnect(this, SIGNAL(ParentEntitySet()), this, SLOT(CreateLaser()));
    DestroyLaser();
}

void EC_LaserPointer::CreateLaser()
{
    if (!ViewEnabled())
        return;
    if (renderer_.expired())
        return;
    if (GetFramework()->IsHeadless())
        return;

    Ogre::SceneManager *scene = renderer_.lock()->GetSceneManager();
    assert(scene);
    if (!scene)
        return;

    Scene::Entity *parentEntity = GetParentEntity();
    assert(parentEntity);
    if (!parentEntity)
        return;

    node_ = parentEntity->GetComponent<EC_Placeable>().get();

    if (!node_)
        return;

    id_ = Ogre::StringConverter::toString((int)parentEntity->GetId());

    laserObject_ = scene->createManualObject("laser" + id_);
    Ogre::SceneNode* laserObjectNode = scene->getRootSceneNode()->createChildSceneNode("laser" + id_ + "_node");
    Ogre::MaterialPtr laserMaterial = Ogre::MaterialManager::getSingleton().create("laser" + id_ + "Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME); 
    laserMaterial->setReceiveShadows(false);
    laserMaterial->getTechnique(0)->setLightingEnabled(true);
    laserMaterial->getTechnique(0)->getPass(0)->setDiffuse(1,0,0,1);
    laserMaterial->getTechnique(0)->getPass(0)->setAmbient(1,0,0);
    laserMaterial->getTechnique(0)->getPass(0)->setSelfIllumination(1,0,0);
    laserObjectNode->attachObject(laserObject_);

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(HandleAttributeChange(IAttribute*, AttributeChange::Type)));

    EC_InputMapper *mapper = parentEntity->GetComponent<EC_InputMapper>().get();
    if (mapper)
    {
        input_ = mapper->GetInputContext();
        connect(input_, SIGNAL(MouseMove(MouseEvent*)), this, SLOT(Update(MouseEvent*)));
    }
}

void EC_LaserPointer::DestroyLaser()
{
    if (!ViewEnabled())
        return;
    if (renderer_.expired())
        return;
    if (GetFramework()->IsHeadless())
        return;

    OgreRenderer::RendererPtr renderer = renderer_.lock();
    Ogre::SceneManager* scene = renderer_.lock()->GetSceneManager();
    if (scene)
    {
        try
        {
            Ogre::SceneNode* node = dynamic_cast<Ogre::SceneNode*> (scene->getRootSceneNode()->getChild("laser" + id_ + "_node"));
            if (node)
                node->detachObject("laser" + id_);
            scene->getRootSceneNode()->removeAndDestroyChild("laser" + id_ + "_node");
            scene->destroyManualObject("laser" + id_);
        }
        catch (...)
        {
        }
    }

    if (renderer)
    {
        try
        {
           Ogre::MaterialManager::getSingleton().remove("laser" + id_ + "Material");
        }
        catch(...)
        {
        }
    }
}

void EC_LaserPointer::Update(MouseEvent *e)
{
    if (IsEnabled() && !GetFramework()->IsHeadless())
    {
        if (canUpdate_)
        {
            OgreRenderer::Renderer *renderer = renderer_.lock().get();
            RaycastResult *result = renderer->Raycast(e->x, e->y);
            if (result->getentity() && result->getentity() != GetParentEntity())
            {
                SetStartPos(node_->GetPosition());
                SetEndPos(result->getpos());
                DisableUpdate();
            }
            else
                laserObject_->clear();
        }
    }
}

void EC_LaserPointer::HandleAttributeChange(IAttribute *attribute, AttributeChange::Type change)
{
    if (!ViewEnabled())
        return;
    if (renderer_.expired())
        return;
    if (GetFramework()->IsHeadless())
        return;

    if (IsEnabled())
    {
        laserObject_->clear();
        laserObject_->begin("laser" + id_ + "Material", Ogre::RenderOperation::OT_LINE_LIST);
        // start position
        laserObject_->position((Ogre::Real)GetStartPos().x, 
                               (Ogre::Real)GetStartPos().y,
                               (Ogre::Real)(GetStartPos().z + 0.6)); 
        // end position
        laserObject_->position((Ogre::Real)GetEndPos().x,
                               (Ogre::Real)GetEndPos().y,
                               (Ogre::Real)GetEndPos().z);
        laserObject_->end();
    }
    else
        laserObject_->clear();
}


void EC_LaserPointer::Enable()
{
    enabled_.Set(true, AttributeChange::Default);
}

void EC_LaserPointer::EnableUpdate()
{
    canUpdate_ = true;
}

void EC_LaserPointer::Disable()
{
    laserObject_->clear();
    enabled_.Set(false, AttributeChange::Default);
}

void EC_LaserPointer::DisableUpdate()
{
    canUpdate_ = false;
    QTimer::singleShot(updateInterval_, this, SLOT(EnableUpdate()));
}

void EC_LaserPointer::SetStartPos(const Vector3df pos)
{
    startPos_.Set(pos, AttributeChange::Default);
}

void EC_LaserPointer::SetEndPos(const Vector3df pos)
{
    endPos_.Set(pos, AttributeChange::Default);
}

Vector3df EC_LaserPointer::GetStartPos() const
{
    return startPos_.Get();
}

Vector3df EC_LaserPointer::GetEndPos() const
{
    return endPos_.Get();
}

bool EC_LaserPointer::IsEnabled()
{
    return enabled_.Get();
}
