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

#include "InputAPI.h"
#include "InputContext.h"
#include "MouseEvent.h"

#include "UiAPI.h"
#include "UiMainWindow.h"
#include "UiGraphicsView.h"

#include <QTimer>

#include "LoggingFunctions.h"
DEFINE_POCO_LOGGING_FUNCTIONS("EC_LaserPointer");

#include <Ogre.h>

EC_LaserPointer::EC_LaserPointer(IModule *module) :
    IComponent(module->GetFramework()),
    startPos(this, "startPosition"),
    endPos(this, "endPosition"),
    color(this, "color", Color(1.0f,0.0f,0.0f,1.0f)),
    enabled(this, "enabled", false),
    tracking(false),
    laserObject_(0),
    canUpdate_(true),
    updateInterval_(20),
    id_("")
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer);

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

    id_ = renderer_.lock()->GetUniqueObjectName("");

    laserObject_ = scene->createManualObject("laser" + id_);
    Ogre::SceneNode* laserObjectNode = scene->getRootSceneNode()->createChildSceneNode("laser" + id_ + "_node");
    laserMaterial_ = Ogre::MaterialManager::getSingleton().create("laser" + id_ + "Material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME); 
    laserMaterial_->setReceiveShadows(false);
    laserMaterial_->getTechnique(0)->setLightingEnabled(true);
    UpdateColor();
    laserObjectNode->attachObject(laserObject_);

    connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(HandleAttributeChange(IAttribute*, AttributeChange::Type)));
    
    input_ = framework_->Input()->RegisterInputContext(QString::fromStdString(id_), 90);
    if (input_.get())
    {
        input_->SetTakeMouseEventsOverQt(true);
        connect(input_.get(), SIGNAL(MouseMove(MouseEvent*)), this, SLOT(Update(MouseEvent*)));
    }
    else
        LogError("Could not register a input context, cannot track mouse movement!");
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
    if (!gettracking())
        return;

    if (getenabled() && !GetFramework()->IsHeadless())
    {
        if (canUpdate_)
        {
            // See if we are inside the main window or there is a graphics item under the mouse
            if (!IsMouseInsideWindow() || IsItemUnderMouse())
            {
                laserObject_->clear();
                return;
            }

            OgreRenderer::Renderer *renderer = renderer_.lock().get();
            RaycastResult *result = renderer->Raycast(e->x, e->y);
            if (result && result->getentity() && result->getentity() != GetParentEntity())
            {
                EC_Placeable *placeable = GetParentEntity()->GetComponent<EC_Placeable>().get();
                if (placeable)
                {
                    Vector3df position = placeable->gettransform().position;
                    if (position != startPos.Get())
                        SetStartPos(position);
                }
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

    if (attribute == &color)
    {
        UpdateColor();
        return;
    }

    if (!laserObject_)
        return;

    if (attribute == &startPos || attribute == &endPos || attribute == &enabled)
    {
        if (getenabled())
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
}

void EC_LaserPointer::HandlePlaceableAttributeChange(IAttribute *attribute, AttributeChange::Type change)
{
    if (attribute->GetNameString() == "Transform")
    {
        if (!ViewEnabled())
            return;
        if (!gettracking() || !getenabled())
            return;
        if (!canUpdate_)
            return;
        if (!GetParentEntity())
            return;
        EC_Placeable *placeable = GetParentEntity()->GetComponent<EC_Placeable>().get();
        if (!placeable)
            return;

        Vector3df position = placeable->gettransform().position;
        if (position != startPos.Get())
            startPos.Set(position, AttributeChange::Default);

        // See if we are inside the main window or there is a graphics item under the mouse
        if (!IsMouseInsideWindow() || IsItemUnderMouse())
        {
            laserObject_->clear();
            return;
        }

        OgreRenderer::Renderer *renderer = renderer_.lock().get();
        if (!renderer)
            return;

        QPoint scenePos = framework_->Ui()->GraphicsView()->mapFromGlobal(QCursor::pos());
        RaycastResult *result = renderer->Raycast(scenePos.x(), scenePos.y());
        if (result && result->getentity() && result->getentity() != GetParentEntity())
        {
            SetEndPos(result->getpos());
            DisableUpdate();
        }
        else
            laserObject_->clear();
    }
}

void EC_LaserPointer::Enable()
{
    enabled.Set(true, AttributeChange::Default);
}

void EC_LaserPointer::EnableUpdate()
{
    canUpdate_ = true;
}

void EC_LaserPointer::Disable()
{
    laserObject_->clear();
    enabled.Set(false, AttributeChange::Default);
}

void EC_LaserPointer::DisableUpdate()
{
    canUpdate_ = false;
    QTimer::singleShot(updateInterval_, this, SLOT(EnableUpdate()));
}

void EC_LaserPointer::SetStartPos(const Vector3df pos)
{
    startPos.Set(pos, AttributeChange::Default);
}

void EC_LaserPointer::SetEndPos(const Vector3df pos)
{
    endPos.Set(pos, AttributeChange::Default);
}

Vector3df EC_LaserPointer::GetStartPos() const
{
    return startPos.Get();
}

Vector3df EC_LaserPointer::GetEndPos() const
{
    return endPos.Get();
}

void EC_LaserPointer::SetQColor(const QColor & c)
{
    Color col = Color((float)c.redF(),
                      (float)c.greenF(),
                      (float)c.blueF(),
                      (float)c.alphaF());

    color.Set(col, AttributeChange::Default);
}

void EC_LaserPointer::SetColor(int red, int green, int blue, int alpha = 255)
{
    QColor color = QColor(red, green, blue, alpha);
    SetQColor(color);
}

Color EC_LaserPointer::GetColor() const
{
    return color.Get();
}

QColor EC_LaserPointer::GetQColor() const
{
    Color c = GetColor();
    QColor newcolor;
    newcolor.setRgbF(c.r, c.g, c.b, c.a);
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

    Color c = GetColor();
    laserMaterial_->getTechnique(0)->getPass(0)->setDiffuse(c.r, c.g, c.b, c.a);
    laserMaterial_->getTechnique(0)->getPass(0)->setAmbient(c.r, c.g, c.b);
    laserMaterial_->getTechnique(0)->getPass(0)->setSelfIllumination(c.r, c.g, c.b);
}

bool EC_LaserPointer::IsMouseInsideWindow()
{
    if (!framework_->Ui()->MainWindow())
        return false;
    return framework_->Ui()->MainWindow()->geometry().contains(QCursor::pos(), true);
}

bool EC_LaserPointer::IsItemUnderMouse()
{
    if (!framework_->Ui()->GraphicsView() || !framework_->Ui()->MainWindow())
        return true;
    QPoint scenePos = framework_->Ui()->GraphicsView()->mapFromGlobal(QCursor::pos());
    QGraphicsItem *itemUnderMouse = framework_->Ui()->GraphicsView()->itemAt(scenePos);
    return (itemUnderMouse ? true : false);
}