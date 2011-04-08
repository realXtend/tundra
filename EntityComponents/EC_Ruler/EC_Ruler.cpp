/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Ruler.cpp
 *  @brief  EC_Ruler provides visualisation for transformations.
 *  @note   The entity must have EC_Placeable component available in advance.
 *
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Ruler.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "LoggingFunctions.h"
#ifdef ENABLE_TAIGA_SUPPORT
#include "RexUUID.h"
#endif
#include "CoreMath.h"
#include <Ogre.h>

#include "MemoryLeakCheck.h"

EC_Ruler::EC_Ruler(IModule *module) :
    IComponent(module->GetFramework()),
    typeAttr_(this, "ruler type", EC_Ruler::Rotation),
    visibleAttr_(this, "visible", false),
    axisAttr_(this, "axis", EC_Ruler::X),
    localAttr_(this, "local", false),
    radiusAttr_(this, "radius", 5),
    segmentsAttr_(this, "segments", 29),
    rulerObject(0),
    gridObject(0),
    sceneNode_(0),
    type(EC_Ruler::Rotation)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Service::ST_Renderer);

#ifdef ENABLE_TAIGA_SUPPORT
    RexUUID uuid = RexUUID::CreateRandom();
    rulerName = uuid.ToString() + "ruler";
    nodeName = uuid.ToString() + "node";
    rulerMovingPartName = uuid.ToString() + "mover";
    movingNodeName = uuid.ToString() + "movingNode";
#else
    rulerName = "ruler";
    nodeName = "node";
    rulerMovingPartName = "mover";
    movingNodeName = "movingNode";
#endif
    
    QObject::connect(this, SIGNAL(AttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(UpdateRuler()));
}

EC_Ruler::~EC_Ruler()
{
    // OgreRendering module might be already deleted. If so, the cloned entity is also already deleted.
    // In this case, just set pointer to 0.
    if (!renderer_.expired())
    {
        Ogre::SceneManager *sceneMgr = renderer_.lock()->GetSceneManager();
        sceneMgr->destroyManualObject(rulerObject);
        if(gridObject)
            sceneMgr->destroyManualObject(gridObject);
    }
    else
    {
        rulerObject = 0;
        gridObject = 0;
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
    
    rulerObject->setVisible(true);
    
    if (gridObject )
        gridObject->setVisible(true);
    
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
    
    rulerObject->setVisible(false);
    
    if (gridObject )
        gridObject->setVisible(false);
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

    EC_Placeable *placeable = entity->GetComponent<EC_Placeable>().get();
    assert(placeable);
    if (!placeable)
        return;
    sceneNode_ = placeable->GetSceneNode();

    assert(sceneNode_);
    if (!sceneNode_)
        return;
    
    if(scene_mgr->hasManualObject(rulerName)){
        rulerObject = scene_mgr->getManualObject(rulerName);
        if(rulerObject->isAttached())
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
            rulerObject->detatchFromParent();
#else
            rulerObject->detachFromParent();
#endif
    } else {
        rulerObject = scene_mgr->createManualObject(rulerName);
    }
    if(scene_mgr->hasManualObject(rulerMovingPartName)){
        gridObject = scene_mgr->getManualObject(rulerMovingPartName);
        if(gridObject->isAttached())
#if OGRE_VERSION_MINOR <= 6 && OGRE_VERSION_MAJOR <= 1
            gridObject->detatchFromParent();
#else
            gridObject->detachFromParent();
#endif
    } else {
        gridObject = scene_mgr->createManualObject(rulerMovingPartName);
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
        if(scene_mgr->hasSceneNode(nodeName)) {
            globalSceneNode = scene_mgr->getSceneNode(nodeName);
        } else {
            globalSceneNode = scene_mgr->getRootSceneNode()->createChildSceneNode(nodeName);
            globalSceneNode->setVisible(true);
        }
        assert(globalSceneNode);
        if(!globalSceneNode)
            return;
            
        if(scene_mgr->hasSceneNode(movingNodeName)) {
            anchorNode = scene_mgr->getSceneNode(movingNodeName);
        } else {
            anchorNode = scene_mgr->getRootSceneNode()->createChildSceneNode(movingNodeName);
            anchorNode->setVisible(true);
        }
        assert(anchorNode);
        if(!anchorNode)
            return;
        
        anchorNode->setPosition(0,0,0);
        anchorNode->attachObject(gridObject);
    
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
    rulerObject->setCastShadows(false);
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
    rulerObject->setCastShadows(false);
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
    
    Quaternion start = Quaternion(rot_.x(), rot_.y(), rot_.z(), rot_.scalar());
    Vector3df seul;
    Quaternion end= Quaternion(newrot_.x(), newrot_.y(), newrot_.z(), newrot_.scalar());
    Vector3df eeul;
    
    start.toEuler(seul);
    end.toEuler(eeul);
    
    float a1 = 0.0f;
    float a2 = 0.0f;
    switch(axisAttr_.Get()) {
        case EC_Ruler::X:
                a2 = seul.x - eeul.x;
            break;
        case EC_Ruler::Y:
                a2 = seul.y - eeul.y;
            break;
        case EC_Ruler::Z:
                a2 = seul.z - eeul.z;
            break;
    }
    
    int dir = 1;
    if (a2 < 0.0f) {
        dir = -1;
    }
    
    for(float theta = 0; theta <= 2 * Ogre::Math::PI; theta += Ogre::Math::PI / segments) {
        switch(axisAttr_.Get()) {
            case EC_Ruler::X:
                rulerObject->position(0, radius * cos(theta), radius * sin(theta));
                break;
            case EC_Ruler::Y:
                rulerObject->position(radius * cos(theta), 0, radius * sin(theta));
                break;
            case EC_Ruler::Z:
                rulerObject->position(radius * cos(theta), radius * sin(theta), 0);
                break;
        }
    }
    rulerObject->end();
    
    float outer_radius = radius + 0.5f;
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);
    rulerObject->position(0,0,0);
    if(dir==1) {
        for(float theta = a1; theta <= a2; theta += Ogre::Math::PI/ segments) {
            switch(axisAttr_.Get()) {
                case EC_Ruler::X:
                    rulerObject->position(0, outer_radius * cos(theta), radius * sin(theta));
                    break;
                case EC_Ruler::Y:
                    rulerObject->position(outer_radius * cos(theta), 0, radius * sin(theta));
                    break;
                case EC_Ruler::Z:
                    rulerObject->position(outer_radius * cos(theta), radius * sin(theta), 0);
                    break;
            }
        } 
    } else {
        for(float theta = a1; theta >= a2; theta -= Ogre::Math::PI/ segments) {
            switch(axisAttr_.Get()) {
                case EC_Ruler::X:
                    rulerObject->position(0, outer_radius * cos(theta), radius * sin(theta));
                    break;
                case EC_Ruler::Y:
                    rulerObject->position(outer_radius * cos(theta), 0, radius * sin(theta));
                    break;
                case EC_Ruler::Z:
                    rulerObject->position(outer_radius * cos(theta), radius * sin(theta), 0);
                    break;
            }
        } 
    }
    rulerObject->position(0,0,0);
    rulerObject->end();
}

void EC_Ruler::SetupTranslateRuler() {
    if(!rulerObject)
        return;

    float size = radiusAttr_.Get();
    float x, y, z;
    int d = 0;
    
    x = y = z = 0.0f;
    
    switch(axisAttr_.Get()) {
        case EC_Ruler::X:
            x = size;
            d = floor(newpos_.x())-floor(pos_.x());
            break;
        case EC_Ruler::Y:
            y = size;
            d = floor(newpos_.y())-floor(pos_.y());
            break;
        case EC_Ruler::Z:
            z = size;
            d = floor(newpos_.z())-floor(pos_.z());
            break;
        default:
            x = size;
            y = size;
    }
    rulerObject->clear();
    rulerObject->setCastShadows(false);
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
    rulerObject->position(x, y, z);
    rulerObject->position(-x, -y, -z);
    rulerObject->end();
    
    // create grid
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
    gridObject->clear();
    gridObject->setCastShadows(false);
    gridObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_LIST);
    
    for(int step=(-5+d); step <= (5+d); step += 1) {
        switch(axisAttr_.Get()) {
            case EC_Ruler::X:
                // side one
                gridObject->position(floor(pos_.x())+(float)step, pos_.y()-1, pos_.z()+0.05f);
                gridObject->position(floor(pos_.x())+(float)step, pos_.y()-1, pos_.z()-0.05f);
                gridObject->position(floor(pos_.x())+(float)step, pos_.y()-1.05f, pos_.z());
                // side two
                gridObject->position(floor(pos_.x())+(float)step, pos_.y()+1.05f, pos_.z());
                gridObject->position(floor(pos_.x())+(float)step, pos_.y()+1, pos_.z()+0.05f);
                gridObject->position(floor(pos_.x())+(float)step, pos_.y()+1, pos_.z()-0.05f);
                break;
            case EC_Ruler::Y:
                // side one
                gridObject->position(pos_.x()-1, floor(pos_.y())+(float)step, pos_.z()+0.05f);
                gridObject->position(pos_.x()-1, floor(pos_.y())+(float)step, pos_.z()-0.05f);
                gridObject->position(pos_.x()-1.05, floor(pos_.y())+(float)step, pos_.z());
                // side two
                gridObject->position(pos_.x()+1.05, floor(pos_.y())+(float)step, pos_.z());
                gridObject->position(pos_.x()+1, floor(pos_.y())+(float)step, pos_.z()+0.05f);
                gridObject->position(pos_.x()+1, floor(pos_.y())+(float)step, pos_.z()-0.05f);
                break;
            case EC_Ruler::Z:
                // side one
                gridObject->position(pos_.x()-1, pos_.y()+0.05f, floor(pos_.z())+step);
                gridObject->position(pos_.x()-1, pos_.y()-0.05f, floor(pos_.z())+step);
                gridObject->position(pos_.x()-1.05, pos_.y(), floor(pos_.z())+step);
                // side two
                gridObject->position(pos_.x()+1.05, pos_.y(), floor(pos_.z())+step);
                gridObject->position(pos_.x()+1, pos_.y()+0.05f, floor(pos_.z())+step);
                gridObject->position(pos_.x()+1, pos_.y()-0.05f, floor(pos_.z())+step);
                break;
        }
    }
    gridObject->end();
    rulerObject->end();
}

void EC_Ruler::StartDrag(QVector3D pos, QQuaternion rot, QVector3D scale)
{
    pos_ = newpos_ = pos;
    rot_ = newrot_ = rot;
    scale_ = newscale_ = scale;
    UpdateRuler();
}

void EC_Ruler::DoDrag(QVector3D pos, QQuaternion rot, QVector3D scale)
{
    newpos_ = pos;
    newrot_ = rot;
    newscale_ = scale;
    UpdateRuler();
}

void EC_Ruler::EndDrag() {
    pos_ = newpos_;
    rot_ = newrot_;
    scale_ = newscale_;
    UpdateRuler();
}

void EC_Ruler::UpdateRuler() {
    Create();
    
    if(visibleAttr_.Get()) {
        Show();
    } else {
        Hide();
    }
}

