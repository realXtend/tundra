/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EC_SoundRuler.h
 *  @brief  EC_SoundRuler enables visual sound properties effect for scene entity.
 *  @note   The entity must have EC_OgrePlaceable and EC_OpenSimPrim 
 *  @author Nathan Letwory | http://www.letworyinteractive.com
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "EC_SoundRuler.h"
#include "ModuleInterface.h"
#include "Entity.h"
#include "Renderer.h"
#include "OgreMaterialUtils.h"
#include "EC_OgrePlaceable.h"
#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_OpenSimPrim.h"
#include "LoggingFunctions.h"
#include <Ogre.h>

DEFINE_POCO_LOGGING_FUNCTIONS("EC_SoundRuler")

#include "MemoryLeakCheck.h"

EC_SoundRuler::EC_SoundRuler(Foundation::ModuleInterface *module) :
    visibleAttr_(this, "visible", true),
    radiusAttr_(this, "radius", 5),
    volumeAttr_(this, "volume", 5),
    segmentsAttr_(this, "segments", 29),
    rulerObject(0),
    sceneNode_(0),
    prim(0)
{
    renderer_ = module->GetFramework()->GetServiceManager()->GetService<OgreRenderer::Renderer>(Foundation::Service::ST_Renderer);
    
    QObject::connect(this, SIGNAL(OnChanged()), this, SLOT(UpdateSoundRuler()));
}

EC_SoundRuler::~EC_SoundRuler()
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

void  EC_SoundRuler::Show()
{
    if (!rulerObject)
        Create();

    if (!rulerObject)
    {
        LogError("EC_SoundRuler not initialized properly.");
        return;
    }

    if (rulerObject)
        rulerObject->setVisible(true);
}

void  EC_SoundRuler::Hide()
{
    if (!rulerObject)
        Create();
        
    if (!rulerObject)
    {
        LogError("EC_SoundRuler not initialized properly.");
        return;
    }
    
    if (rulerObject)
        rulerObject->setVisible(false);
}

bool EC_SoundRuler::IsVisible() const
{
    if (rulerObject)
        return rulerObject->isVisible();

    return false;
}

void EC_SoundRuler::Create()
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
    
    prim = entity->GetComponent<EC_OpenSimPrim>().get();
    assert(prim);
    if (!prim)
        return;

    assert(sceneNode_);
    if (!sceneNode_)
        return;
    
    if(scene_mgr->hasManualObject("soundRuler")) {
        rulerObject = scene_mgr->getManualObject("soundRuler");
        if(rulerObject->isAttached())
            rulerObject->detatchFromParent();
    } else {
        rulerObject = scene_mgr->createManualObject("soundRuler");
    }
    
    SetupSoundRuler();

    // get soundRulerNode only when we are working in world space
    if(scene_mgr->hasSceneNode("soundRulerNode")) {
        globalSceneNode = scene_mgr->getSceneNode("soundRulerNode");
    } else {
        globalSceneNode = scene_mgr->getRootSceneNode()->createChildSceneNode("soundRulerNode");
        globalSceneNode->setVisible(true);
    }
    assert(globalSceneNode);
    if(!globalSceneNode)
        return;

    globalSceneNode->setPosition(sceneNode_->getParent()->getPosition());
    globalSceneNode->attachObject(rulerObject);
}

void EC_SoundRuler::SetupSoundRuler()
{
    if(!rulerObject || !prim)
        return;
    float const radius = prim->SoundRadius;//radiusAttr_.Get();
    float const segments = segmentsAttr_.Get();

    rulerObject->clear();
    rulerObject->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_LINE_STRIP);

    unsigned i = 0;
    for(float theta = 0; theta <= 2 * Ogre::Math::PI; theta += Ogre::Math::PI / segments) {
        rulerObject->position(radius * cos(theta), radius * sin(theta), 0);
        rulerObject->index(i++);
    }
    rulerObject->index(0); // Close the line = circle
    rulerObject->end();
}

void EC_SoundRuler::UpdateSoundRuler() {
    Create();
    
    if(visibleAttr_.Get()) {
        Show();
    } else {
        Hide();
    }
}

