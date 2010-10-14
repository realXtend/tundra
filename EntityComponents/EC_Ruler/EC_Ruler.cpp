/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_Ruler.cpp
 *  @brief  EC_Ruler enables visual highlighting effect for of scene entity.
 *  @note   The entity must have EC_OgrePlaceable and EC_Mesh (if mesh) or
 *          EC_OgreCustomObject (if prim) components available in advance.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_Ruler.h"
#include "IModule.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_OgrePlaceable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "LoggingFunctions.h"
#include "RexUUID.h"
#include "CoreMath.h"
#include <Ogre.h>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Ruler")

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
    sceneNode_(0),
    type(EC_Ruler::Rotation)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    
    RexUUID uuid = RexUUID::CreateRandom();
    rulerName = uuid.ToString() + "ruler";
    nodeName = uuid.ToString() + "node";
    
    QObject::connect(this, SIGNAL(OnAttributeChanged(IAttribute*, AttributeChange::Type)), this, SLOT(UpdateRuler()));
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
                //a1 = seul.x;
                a2 = seul.x - eeul.x;
            break;
        case EC_Ruler::Y:
                //a1 = seul.y;
                a2 = seul.y - eeul.y;
            break;
        case EC_Ruler::Z:
                //a1 = seul.z;
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
    float x, y, z, p, delta;
    
    x = y = z = p = 0;
    
    switch(axisAttr_.Get()) {
        case EC_Ruler::X:
            x = size;
            p = fmodf(abs(pos_.x()), 1.0f);
            delta = pos_.x()-newpos_.x();
            break;
        case EC_Ruler::Y:
            p = fmodf(abs(pos_.y()), 1.0f);
            delta = pos_.y()-newpos_.y();
            y = size;
            break;
        case EC_Ruler::Z:
            p = fmodf(abs(pos_.z()), 1.0f);
            delta = pos_.z()-newpos_.z();
            z = size;
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
    float crossp;
    for(int step=-5; step <= 5; step += 1) {
        crossp = (float)step + p + delta;
        switch(axisAttr_.Get()) {
            case EC_Ruler::X:
                // side one
                rulerObject->position(crossp, -1, 0.05f);
                rulerObject->position(crossp, -1, -0.05f);
                if(abs(crossp) > 0.5f)
                    rulerObject->position(crossp, -0.95f, 0);
                rulerObject->position(crossp, -1.05f, 0);
                // side two
                if(abs(crossp) > 0.5f)
                    rulerObject->position(crossp, 0.95f, 0);
                rulerObject->position(crossp, 1.05f, 0);
                rulerObject->position(crossp, 1, 0.05f);
                rulerObject->position(crossp, 1, -0.05f);
                break;
            case EC_Ruler::Y:
                // side one
                rulerObject->position(-1, crossp, 0.05f);
                rulerObject->position(-1, crossp, -0.05f);
                if(abs(crossp) > 0.5f)
                    rulerObject->position(-0.95f, crossp, 0);
                rulerObject->position(-1.05f, crossp, 0);
                // side two
                if(abs(crossp) > 0.5f)
                    rulerObject->position(0.95f, crossp, 0);
                rulerObject->position(1.05f, crossp, 0);
                rulerObject->position(1, crossp, 0.05f);
                rulerObject->position(1, crossp, -0.05f);
                break;
            case EC_Ruler::Z:
                // side one
                rulerObject->position(-1, 0.05f, crossp);
                rulerObject->position(-1, -0.05f, crossp);
                if(abs(crossp) > 0.5f)
                    rulerObject->position(-0.95f, 0, crossp);
                rulerObject->position(-1.05f, 0, crossp);
                // side two
                if(abs(crossp) > 0.5f)
                    rulerObject->position(0.95f, 0, crossp);
                rulerObject->position(1.05f, 0, crossp);
                rulerObject->position(1, 0.05f, crossp);
                rulerObject->position(1, -0.05f, crossp);
                break;
        }
    }
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

