// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QOgreWorldView.h"
#include "Profiler.h"

namespace OgreRenderer
{
    QOgreWorldView::QOgreWorldView(Ogre::RenderWindow *win) : win_(win), texture_name_("test/texture/UI")
    {
        root_ = Ogre::Root::getSingletonPtr();
    }

    QOgreWorldView::~QOgreWorldView()
    {
    }

    void QOgreWorldView::InitializeOverlay(int width, int height)
    {
        // set up off-screen texture
        Ogre::TexturePtr ui_overlay_texture_ = Ogre::TextureManager::getSingleton().createManual(
            texture_name_, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
            Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

        Ogre::MaterialPtr material(Ogre::MaterialManager::getSingleton().create(
            "test/material/UI", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));

        Ogre::TextureUnitState *state(material->getTechnique(0)->getPass(0)->createTextureUnitState());

        state->setTextureName(texture_name_);

        material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        // Setup fog override so that scene fog does not affect UI rendering
        material->setFog(true, Ogre::FOG_NONE);

        // set up overlays
        ui_overlay_ = Ogre::OverlayManager::getSingleton().create("test/overlay/UI");

        ui_overlay_container_ = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "test/overlay/UIPanel");

        ui_overlay_container_->setMaterialName("test/material/UI");
        ui_overlay_container_->setMetricsMode(Ogre::GMM_PIXELS);
        ui_overlay_container_->setPosition(0, 0);

        ui_overlay_->add2D(static_cast<Ogre::OverlayContainer *>(ui_overlay_container_));
        ui_overlay_->setZOrder(500);
        ui_overlay_->show();

        ResizeOverlay(width, height);
    }

    void QOgreWorldView::ResizeWindow(int width, int height)
    {
        if (win_)
        {
            PROFILE(QOgreWorldView_ResizeWindow);
			//$ BEGIN_MOD $
			//$ MOD_DESCRIPTION Comment this to make it render in the correct place with the new QMainWindow $
            //win_->resize(width, height); 
			//$ END_MOD $
            win_->windowMovedOrResized();
        }
    }

    void QOgreWorldView::ResizeOverlay(int width, int height)
    {
        if (Ogre::TextureManager::getSingletonPtr() && Ogre::OverlayManager::getSingletonPtr())
        {
            PROFILE(QOgreWorldView_ResizeOverlay);
            
            // recenter the overlay
            int left = (win_->getWidth() - width) / 2;
            int top = (win_->getHeight() - height) / 2;

            // resize the container
            ui_overlay_container_->setDimensions(width, height);
            ui_overlay_container_->setPosition(left, top);

            // resize the backing texture
            Ogre::TextureManager &mgr = Ogre::TextureManager::getSingleton();
            Ogre::TexturePtr texture = mgr.getByName(texture_name_);
            assert(texture.get());

            texture->freeInternalResources();
            texture->setWidth(width);
            texture->setHeight(height);
            texture->createInternalResources();
        }
    }

    void QOgreWorldView::RenderOneFrame()
    {
        PROFILE(QOgreWorldView_RenderOneFrame);
        //root_->_fireFrameStarted();
        //win_-> update();
        //root_->_fireFrameRenderingQueued();
        //root_->_fireFrameEnded();
        root_->renderOneFrame();
    }

    void QOgreWorldView::OverlayUI(Ogre::PixelBox &ui)
    {
        PROFILE(QOgreWorldView_OverlayUI);
        Ogre::TextureManager &mgr = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr texture = mgr.getByName(texture_name_);
        assert(texture.get());
        if (!texture->getBuffer().isNull())
            texture->getBuffer()->blitFromMemory(ui);
    }

    void QOgreWorldView::ShowUiOverlay()
    {
        ui_overlay_->show();
        //RenderOneFrame();
    }

    void QOgreWorldView::HideUiOverlay()
    {
        ui_overlay_->hide();
        //RenderOneFrame();
    }
}
