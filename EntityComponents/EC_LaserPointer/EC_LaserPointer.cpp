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
#include "SceneManager.h"

#include <QTimer>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_LaserPointer");

#include <Ogre.h>

EC_LaserPointer::EC_LaserPointer(IModule *module) :
    IComponent(module->GetFramework()),
    startPos_(this, "startPosition"),
    endPos_(this, "endPosition"),
    color_(this, "color", Color(1.0f,0.0f,0.0f,1.0f)),
    enabled_(this, "enabled", false),
    laserObject_(0),
    canUpdate_(true),
    updateInterval_(20),
    input_(0),
    id_()
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer);

    AttributeMetadata *meta = new AttributeMetadata();
    meta->designable = false;
    startPos_.SetMetadata(meta);
    endPos_.SetMetadata(meta);

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
    if (!scene)
        return;

    Scene::Entity *parentEntity = GetParentEntity();
    if (!parentEntity)
        return;

    EC_Placeable *placeable = parentEntity->GetComponent<EC_Placeable>().get();
    if (placeable)
        connect(placeable, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(HandlePlaceableAttributeChange(IAttribute*, AttributeChange::Type)));
    else
        LogWarning("Placeable is not preset, cannot connect to position changes!");

    id_ = Ogre::StringConverter::toString((int)parentEntity->GetId());

    laserObject_ = scene->createManualObject("laser" + id_);
    Ogre::SceneNode* laserObjectNode = scene->getRootSceneNode()->createChildSceneNode("laser" + id_ + "_node");
    laserMaterial_ = Ogre::MaterialManager::getSingleton().create("laser" + id_ + "Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME); 
    laserMaterial_->setReceiveShadows(false);
    laserMaterial_->getTechnique(0)->setLightingEnabled(true);
    UpdateColor();
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
    if (!GetParentEntity())
        return;

    if (IsEnabled() && !GetFramework()->IsHeadless())
    {
        if (canUpdate_)
        {
            EC_Placeable *placeable = GetParentEntity()->GetComponent<EC_Placeable>().get();
            if (!placeable)
                return;

            OgreRenderer::Renderer *renderer = renderer_.lock().get();
            RaycastResult *result = renderer->Raycast(e->x, e->y);
            if (result->getentity() && result->getentity() != GetParentEntity())
            {
                SetStartPos(placeable->GetPosition());
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

    if (QString::fromStdString(attribute->GetNameString()) == QString::fromStdString(color_.GetNameString()))
    {
        UpdateColor();
        return;
    }

    if (!laserObject_)
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

void EC_LaserPointer::HandlePlaceableAttributeChange(IAttribute *attribute, AttributeChange::Type change)
{
    if (attribute->GetNameString() == "Transform")
    {
        if (!ViewEnabled())
            return;
        if (!IsEnabled())
            return;
        if (!canUpdate_)
            return;
        if (!GetParentEntity())
            return;
        EC_Placeable *placeable = GetParentEntity()->GetComponent<EC_Placeable>().get();
        if (!placeable)
            return;

        Vector3df position = placeable->gettransform().position;
        if (position != startPos_.Get())
            startPos_.Set(position, AttributeChange::Default);
    }
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

void EC_LaserPointer::SetQColor(const QColor & color)
{
    Color col = Color((float)color.redF(),
                      (float)color.greenF(),
                      (float)color.blueF(),
                      (float)color.alphaF());

    color_.Set(col, AttributeChange::Default);
}

void EC_LaserPointer::SetColor(int red, int green, int blue, int alpha = 255)
{
    QColor color = QColor(red, green, blue, alpha);
    SetQColor(color);
}

Color EC_LaserPointer::GetColor() const
{
    return color_.Get();
}

QColor EC_LaserPointer::GetQColor() const
{
    Color color = GetColor();
    QColor newcolor;
    newcolor.setRgbF(color.r, color.g, color.b, color.a);
    return newcolor;
}

void EC_LaserPointer::UpdateColor()
{
    if (!ViewEnabled())
        return;
    if (renderer_.expired())
        return;
    if (GetFramework()->IsHeadless())
        return;
    if (laserMaterial_.isNull())
        return;

    Color color = GetColor();
    laserMaterial_->getTechnique(0)->getPass(0)->setDiffuse(color.r,color.g,color.b,color.a);
    laserMaterial_->getTechnique(0)->getPass(0)->setAmbient(color.r,color.g,color.b);
    laserMaterial_->getTechnique(0)->getPass(0)->setSelfIllumination(color.r,color.g,color.b);
}
