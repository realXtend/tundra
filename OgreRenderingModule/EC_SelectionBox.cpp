// For conditions of distribution and use, see copyright notice in license.txt
// Author: Nathan Letwory <nathan@letworyinteractive.com>

#include "StableHeaders.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_SelectionBox.h"
#include "LoggingFunctions.h"
#include "OgreConversionUtils.h"
#include <Ogre.h>

using namespace OgreRenderer;

DEFINE_POCO_LOGGING_FUNCTIONS("EC_Ruler")

#include "MemoryLeakCheck.h"


EC_SelectionBox::EC_SelectionBox(IModule* module) :
    IComponent(module->GetFramework()),
    renderer_(checked_static_cast<OgreRenderingModule*>(module)->GetRenderer()),
    selectionBox_(0)
{
    RendererPtr renderer = renderer_.lock();
    Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
    selectionBox_ = scene_mgr->createManualObject(renderer->GetUniqueObjectName());
    selectionBox_->setRenderQueueGroup(Ogre::RENDER_QUEUE_OVERLAY);
    selectionBox_->setUseIdentityProjection(true);
    selectionBox_->setUseIdentityView(true);
    selectionBox_->setQueryFlags(0);
    scene_mgr->getRootSceneNode()->createChildSceneNode()->attachObject(selectionBox_);
}

EC_SelectionBox::~EC_SelectionBox()
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
    
    if (selectionBox_)
    {
        Ogre::SceneManager* scene_mgr = renderer->GetSceneManager();
        scene_mgr->destroyManualObject(selectionBox_);
        selectionBox_ = 0;
    }
}

void EC_SelectionBox::SetBoundingBox(QRect &view)
{
    if (renderer_.expired())
        return;
    RendererPtr renderer = renderer_.lock();
    Ogre::RenderWindow *renderWindow = renderer->GetCurrentRenderWindow();
    float w= (float)renderWindow->getWidth();
    float h= (float)renderWindow->getHeight();
    float left = (float)(view.left()) / w, right = (float)(view.right()) / w;
    float top = (float)(view.top()) / h, bottom = (float)(view.bottom()) / h;
    
    if(left > right) { float tmp; tmp = left; left = right; right = tmp; }
    if(top > bottom) { float tmp; tmp = top; top = bottom; bottom = tmp; }
    // don't do selection box if too small
    if((right - left) * (bottom-top) < 0.0001) return;
    
    // correct coordinates for overlay
    left = left * 2 - 1;
    right = right * 2 - 1;
    top = 1 - top * 2;
    bottom = 1 - bottom * 2;
 
    selectionBox_->clear();
    selectionBox_->begin("",Ogre::RenderOperation::OT_TRIANGLE_STRIP);
    /*selectionBox_->position(left, top, -1);
    selectionBox_->position(right, top, -1);
    selectionBox_->position(right, bottom, -1);
    selectionBox_->position(left, bottom, -1);
    selectionBox_->position(left, top, -1);*/
    selectionBox_->position(left, top, -1);
    selectionBox_->position(right, top, -1);
    selectionBox_->position(right, bottom, -1);
    selectionBox_->position(left, bottom, -1);
    selectionBox_->end();
    
    selectionBox_->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
}

void EC_SelectionBox::Show()
{
    selectionBox_->clear();
    selectionBox_->setVisible(true);
}

void EC_SelectionBox::Hide()
{
    selectionBox_->clear();
    selectionBox_->setVisible(false);
}


