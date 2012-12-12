/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_LaserPointer.cpp
 *  @brief  Adds laser pointer to entity.
 */

#define MATH_OGRE_INTEROP
//#include "DebugOperatorNew.h"

#include "EC_LaserPointer.h"

#include "Framework.h"
#include "AttributeMetadata.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "InputAPI.h"
#include "InputContext.h"
#include "MouseEvent.h"
#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UiGraphicsView.h"
#include "LoggingFunctions.h"
#include "OgreWorld.h"

#include <QTimer>
#include <Ogre.h>

//#include "MemoryLeakCheck.h"

EC_LaserPointer::EC_LaserPointer(Scene *scene) :
    IComponent(scene),
    startPos(this, "Start position"),
    endPos(this, "End position"),
    color(this, "Color", Color(1.0f,0.0f,0.0f,1.0f)),
    enabled(this, "Enabled", false),
    laserObject_(0),
    canUpdate_(true),
    updateInterval_(20),
    id_(""),
    tracking(false)
{
    world_ = scene->GetWorld<OgreWorld>();

    static AttributeMetadata nonDesignableAttrData;
    static bool metadataInitialized = false;
    if(!metadataInitialized)
    {
        nonDesignableAttrData.designable = false;
        metadataInitialized = true;
    }
    startPos.SetMetadata(&nonDesignableAttrData);
    endPos.SetMetadata(&nonDesignableAttrData);

    connect(this, SIGNAL(ParentEntitySet()), this, SLOT(CreateLaser()));
}

EC_LaserPointer::~EC_LaserPointer()
{
    DestroyLaser();
}

bool EC_LaserPointer::IsVisible() const
{
    if (laserObject_)
        return laserObject_->isVisible();
    return false;
}

void EC_LaserPointer::CreateLaser()
{
    if (!ViewEnabled())
        return;
    if (world_.expired())
        return;
    if (laserObject_)
    {
        LogError("EC_LaserPointer::CreateLaser: Laser pointer already created.");
        return;
    }
    Entity *parentEntity = ParentEntity();
    if (!parentEntity)
        return;
    EC_Placeable *placeable = parentEntity->GetComponent<EC_Placeable>().get();
    if (placeable)
        connect(placeable, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)),
            this, SLOT(HandlePlaceableAttributeChange(IAttribute*, AttributeChange::Type)));
    else
        LogWarning("Placeable is not preset, cannot connect to position changes!");

    try
    {
        Ogre::SceneManager *scene = world_.lock()->OgreSceneManager();
        id_ = world_.lock()->Renderer()->GetUniqueObjectName("laser");
        laserObject_ = scene->createManualObject(id_);
        Ogre::SceneNode* laserObjectNode = scene->getRootSceneNode()->createChildSceneNode(id_ + "_node");
        laserMaterial_ = Ogre::MaterialManager::getSingleton().create(id_ + "Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME); 
        laserMaterial_->setReceiveShadows(false);
        laserMaterial_->getTechnique(0)->setLightingEnabled(true);
        UpdateColor();
        laserObjectNode->attachObject(laserObject_);

        input_ = framework->Input()->RegisterInputContext(QString::fromStdString(id_), 90);
        input_->SetTakeMouseEventsOverQt(true);
        connect(input_.get(), SIGNAL(MouseMove(MouseEvent*)), this, SLOT(Update(MouseEvent*)));
    }
    catch(const Ogre::Exception &ex)
    {
        LogError("EC_LaserPointer::CreateLaser: an expection occurred: " + std::string(ex.what()));
    }
}

void EC_LaserPointer::DestroyLaser()
{
    if (!ViewEnabled())
        return;
    if (world_.expired())
        return;

    try
    {
        Ogre::SceneManager* scene = world_.lock()->OgreSceneManager();
        Ogre::SceneNode* node = dynamic_cast<Ogre::SceneNode*>(scene->getRootSceneNode()->getChild(id_ + "_node"));
        if (node)
            node->detachObject(id_);
        scene->getRootSceneNode()->removeAndDestroyChild(id_ + "_node");
        scene->destroyManualObject(id_);
    }
    catch (...) { }

    try
    {
       Ogre::MaterialManager::getSingleton().remove(id_ + "Material");
    }
    catch(...) { }

    laserObject_ = 0;
}

void EC_LaserPointer::Update(MouseEvent *e)
{
    if (!ViewEnabled())
        return;
    if (!ParentEntity())
        return;
    if (!tracking)
        return;
    if (world_.expired())
        return;

    if (enabled.Get())
    {
        if (canUpdate_)
        {
            // If mouse cursor is visible (not in first-person mode), see if we are inside the main window or there is a graphics item under the mouse
            if (framework->Input()->IsMouseCursorVisible() && (!IsMouseInsideWindow() || IsItemUnderMouse()))
            {
                laserObject_->clear();
                laserObject_->setVisible(false);
                return;
            }

            laserObject_->setVisible(true);
            RaycastResult *result = world_.lock()->Renderer()->Raycast(e->x, e->y);
            if (result && result->entity && result->entity != ParentEntity())
            {
                EC_Placeable *placeable = ParentEntity()->GetComponent<EC_Placeable>().get();
                if (placeable)
                {
                    float3 position = placeable->WorldPosition();
                    if (!position.Equals(startPos.Get()))
                        startPos.Set(position, AttributeChange::Default);
                }
                endPos.Set(result->pos, AttributeChange::Default);
                DisableUpdate();
            }
            else
            {
                laserObject_->clear();
                laserObject_->setVisible(false);
            }
        }
    }
}

void EC_LaserPointer::AttributesChanged()
{
    if (!ViewEnabled())
        return;
    if (world_.expired())
        return;
    if (!laserObject_)
        return;

    if (color.ValueChanged())
    {
        UpdateColor();
        return;
    }
    if (startPos.ValueChanged() || endPos.ValueChanged() || enabled.ValueChanged())
    {
        if (enabled.Get())
        {
            laserObject_->clear();
            laserObject_->begin(id_ + "Material", Ogre::RenderOperation::OT_LINE_LIST);
            laserObject_->position(startPos.Get());
            laserObject_->position(endPos.Get());
            laserObject_->end();
            laserObject_->setVisible(true);
        }
        else
        {
            laserObject_->clear();
            laserObject_->setVisible(false);
        }
    }
}

void EC_LaserPointer::HandlePlaceableAttributeChange(IAttribute *attribute, AttributeChange::Type change)
{
    if (attribute->Name() == "Transform") ///< \todo attribute name string comparison is risky - what if the name changes for some reason?
    {
        if (!ViewEnabled())
            return;
        if (world_.expired())
            return;
        if (!tracking || !enabled.Get())
            return;
        if (!canUpdate_)
            return;
        if (!ParentEntity())
            return;
        EC_Placeable *placeable = ParentEntity()->GetComponent<EC_Placeable>().get();
        if (!placeable)
            return;

        float3 position = placeable->WorldPosition();
        if (!position.Equals(startPos.Get()))
            startPos.Set(position, AttributeChange::Default);

        // If mouse cursor is visible (not in first-person mode), see if we are inside the main window or there is a graphics item under the mouse
        if (framework->Input()->IsMouseCursorVisible() && (!IsMouseInsideWindow() || IsItemUnderMouse()))
        {
            laserObject_->clear();
            laserObject_->setVisible(false);
            return;
        }

        QPoint scenePos = framework->Ui()->GraphicsView()->mapFromGlobal(QCursor::pos());
        RaycastResult *result = world_.lock()->Renderer()->Raycast(scenePos.x(), scenePos.y());
        if (result && result->entity && result->entity != ParentEntity())
        {
            endPos.Set(result->pos, AttributeChange::Default);
            DisableUpdate();
        }
        else
        {
            laserObject_->clear();
            laserObject_->setVisible(false);
        }
    }
}

void EC_LaserPointer::EnableUpdate()
{
    canUpdate_ = true;
}

void EC_LaserPointer::DisableUpdate()
{
    canUpdate_ = false;
    QTimer::singleShot(updateInterval_, this, SLOT(EnableUpdate()));
}

void EC_LaserPointer::UpdateColor()
{
    if (!ViewEnabled())
        return;
    if (world_.expired())
        return;
    if (laserMaterial_.isNull())
        return;

    laserMaterial_->getTechnique(0)->getPass(0)->setDiffuse(color.Get());
    laserMaterial_->getTechnique(0)->getPass(0)->setAmbient(color.Get());
    laserMaterial_->getTechnique(0)->getPass(0)->setSelfIllumination(color.Get());
}

bool EC_LaserPointer::IsMouseInsideWindow() const
{
    if (!framework->Ui()->MainWindow())
        return false;
    return framework->Ui()->MainWindow()->geometry().contains(QCursor::pos(), true);
}

bool EC_LaserPointer::IsItemUnderMouse() const
{
    if (!framework->Ui()->GraphicsView() || !framework->Ui()->MainWindow())
        return true;
    QPoint scenePos = framework->Ui()->GraphicsView()->mapFromGlobal(QCursor::pos());
    QGraphicsItem *itemUnderMouse = framework->Ui()->GraphicsView()->itemAt(scenePos);
    return (itemUnderMouse != 0 ? true : false);
}
