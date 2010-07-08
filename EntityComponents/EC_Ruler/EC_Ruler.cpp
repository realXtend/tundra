/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Ruler.cpp
 *  @brief  EC_Ruler enables visual highlighting effect for of scene entity.
 *  @note   The entity must have EC_OgrePlaceable and EC_OgreMesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Ruler.h"
#include "ModuleInterface.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"
#include "LoggingFunctions.h"
#include <Ogre.h>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Ruler")

#include "MemoryLeakCheck.h"

EC_Ruler::EC_Ruler(Foundation::ModuleInterface *module) :
    typeAttr_(this, "ruler type", EC_Ruler::Rotation),
    visibleAttr_(this, "visible", false),
    axisAttr_(this, "axis", EC_Ruler::X),
    localAttr_(this, "local", false),
    radiusAttr_(this, "radius", 5),
    segmentsAttr_(this, "segments", 29),
    rulerObject(0),
    sceneNode_(0),
    type(EC_Ruler::Rotation)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    
    QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateRuler()));
}

EC_Ruler::~EC_Ruler()
{
    // OgreRendering module might be already deleted. If so, the cloned entity is also already deleted.
    // In this case, just set pointer to 0.
    if (!renderer_.expired())
    {
        Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
            sceneMgr->destroyManualObject(rulerObject);
    }
    else
    {
        rulerObject = 0;
        sceneNode_ = 0;
    }
}

void  EC_Ruler::Show()
{
    if (!rulerObject)
        Create();

    if (!rulerObject)
    {
        LogError("EC_Ruler not initialized properly.");
        return;
    }

    if (rulerObject)
        rulerObject->setVisible(true);
}

void  EC_Ruler::Hide()
{
    if (!rulerObject)
        Create();
        
    if (!rulerObject)
    {
        LogError("EC_Ruler not initialized properly.");
        return;
    }
    
    if (rulerObject)
        rulerObject->setVisible(false);
}

bool EC_Ruler::IsVisible() const
{
    if (rulerObject)
        return rulerObject->isVisible();

    return false;
}

void EC_Ruler::Create()
{
    if (renderer_.expired())
        return;

    Ogre::SceneManager *scene_mgr= renderer_.lock()->GetSceneManager();
    assert(scene_mgr);
    if (!scene_mgr)
        return;

    Scene::Entity *entity = GetParentEntity();
    assert(entity);
    if (!entity)
        return;

    OgreRenderer::EC_OgrePlaceable *placeable = entity->GetComponent<OgreRenderer::EC_OgrePlaceable>().get();
    assert(placeable);
    if (!placeable)
        return;
    sceneNode_ = placeable->GetSceneNode();

    assert(sceneNode_);
    if (!sceneNode_)
        return;
    
    if(scene_mgr->hasManualObject("translateRuler")) {
        rulerObject = scene_mgr->getManualObject("translateRuler");
        if(rulerObject->isAttached())
            rulerObject->detatchFromParent();
    } else {
        rulerObject = scene_mgr->createManualObject("translateRuler");
    }
    
    switch(typeAttr_.Get()) {
        case EC_Ruler::Rotation:
            SetupRotationRuler();
            break;
        case EC_Ruler::Translation:
            SetupTranslateRuler();
            break;
        case EC_Ruler::Scale:
            SetupScaleRuler();
            break;
    }

    if(localAttr_.Get()) {
        sceneNode_->attachObject(rulerObject);
    } else {
        // get translateNode only when we are working in world space
        if(scene_mgr->hasSceneNode("translateNode")) {
            globalSceneNode = scene_mgr->getSceneNode("translateNode");
        } else {
            globalSceneNode = scene_mgr->getRootSceneNode()->createChildSceneNode("translateNode");
            globalSceneNode->setVisible(true);
        }
        assert(globalSceneNode);
        if(!globalSceneNode)
            return;
    
        globalSceneNode->setPosition(sceneNode_->getParent()->getPosition());
        globalSceneNode->attachObject(rulerObject);
    }
}

void EC_Ruler::SetupScaleRuler()
{
    if(!rulerObject)
        return;
        
    float x, y, z;
    x = y = z = 0;
    
    float size = radiusAttr_.Get();

    switch(axisAttr_.Get()) {
        case EC_Ruler::X:
            x = size;
            break;
        case EC_Ruler::Y:
            y = size;
            break;
        case EC_Ruler::Z:
            z = size;
            break;
        default:
            x = size;
            y = size;
    }
    rulerObject->clear();
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
    rulerObject->position(0, 0, 0);
    rulerObject->position(x, y, z);
    rulerObject->end();
}

void EC_Ruler::SetupRotationRuler()
{
    if(!rulerObject)
        return;
    float const radius = radiusAttr_.Get();
    float const segments = segmentsAttr_.Get();

    rulerObject->clear();
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);

    unsigned i = 0;
    for(float theta = 0; theta <= 2 * Ogre::Math::PI; theta += Ogre::Math::PI / segments) {
        switch(axisAttr_.Get()) {
            case EC_Ruler::X:
                rulerObject->position(0, radius * cos(theta), radius * sin(theta));
                break;
            case EC_Ruler::Y:
                rulerObject->position(radius * cos(theta), radius * sin(theta), 0);
                break;
            case EC_Ruler::Z:
                rulerObject->position(radius * cos(theta), 0, radius * sin(theta));
                break;
        }
        rulerObject->index(i++);
    }
    rulerObject->index(0); // Close the line = circle
    rulerObject->end();
}

void EC_Ruler::SetupTranslateRuler() {
    if(!rulerObject)
        return;

    float x, y, z;
    x = y = z = 0;
    
    float size = radiusAttr_.Get();

    // Note, this arbitrary order is result from Py Code
    // TODO: Fix this to something more unified (0 = x, 1 = y, 2 = z) throughout
    // manipulator widget code
    switch(axisAttr_.Get()) {
        case EC_Ruler::X:
            x = size;
            break;
        case EC_Ruler::Y:
            y = size;
            break;
        case EC_Ruler::Z:
            z = size;
            break;
        default:
            x = size;
            y = size;
    }
    rulerObject->clear();
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
    rulerObject->position(x, y, z);
    rulerObject->position(-x, -y, -z);
    rulerObject->end();
}

void EC_Ruler::SetType(EC_Ruler::Type type) {
}

void EC_Ruler::StartDrag() {}

void EC_Ruler::EndDrag() {}

void EC_Ruler::UpdateRuler() {
    Create();
    
    if(visibleAttr_.Get()) {
        Show();
    } else {
        Hide();
    }
}

