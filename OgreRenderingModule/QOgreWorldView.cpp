// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QOgreWorldView.h"

namespace OgreRenderer
{
    QOgreWorldView::QOgreWorldView(Ogre::RenderWindow *win) : win_(win)
    {
        root_ = Ogre::Root::getSingletonPtr();
    }

    QOgreWorldView::~QOgreWorldView()
    {
    }

    void QOgreWorldView::InitializeOverlay(int width, int height)
    {
        // set up off-screen texture
        ui_overlay_texture_ = Ogre::TextureManager::getSingleton().createManual(
            "test/texture/UI", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
             Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

        Ogre::MaterialPtr material(Ogre::MaterialManager::getSingleton().create(
            "test/material/UI", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME));

        Ogre::TextureUnitState *state(material->getTechnique(0)->getPass(0)->createTextureUnitState());

        state->setTextureName ("test/texture/UI");

        material->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
        // Setup fog override so that scene fog does not affect UI rendering
        material->setFog(true, Ogre::FOG_NONE);

        // set up overlays
        ui_overlay_ = (Ogre::OverlayManager::getSingleton().create("test/overlay/UI"));

        ui_overlay_container_ = (Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "test/overlay/UIPanel"));

        ui_overlay_container_->setMaterialName("test/material/UI");
        ui_overlay_container_->setMetricsMode(Ogre::GMM_PIXELS);
        ui_overlay_container_-> setPosition (0, 0);

        ui_overlay_->add2D(static_cast <Ogre::OverlayContainer *>(ui_overlay_container_));
        ui_overlay_->setZOrder(Ogre::ushort(500));
        ui_overlay_->show();

        ResizeOverlay(width, height);
    }

    void QOgreWorldView::ResizeWindow(int width, int height)
    {
        if (win_)
        {
            PROFILE(QOgreWorldView_ResizeWindow);
            win_->resize(width, height); 
            win_->windowMovedOrResized();
        }
    }

    void QOgreWorldView::ResizeOverlay(int width, int height)
    {
        if (Ogre::TextureManager::getSingletonPtr() && Ogre::OverlayManager::getSingletonPtr())
        {
            PROFILE(QOgreWorldView_ResizeOverlay);

            // resize the container
            ui_overlay_container_-> setDimensions (width, height);

            // resize the backing texture
            ui_overlay_texture_->freeInternalResources();
            ui_overlay_texture_->setWidth(width);
            ui_overlay_texture_->setHeight(height);
            ui_overlay_texture_->createInternalResources();
        }
    }

    void QOgreWorldView::RenderOneFrame()
    {
        PROFILE(QOgreWorldView_RenderOneFrame);
        root_->_fireFrameStarted();
        win_-> update();
        root_->_fireFrameRenderingQueued();
        root_->_fireFrameEnded();
    }

    void QOgreWorldView::OverlayUI(Ogre::PixelBox &ui)
    {
        PROFILE(QOgreWorldView_OverlayUI);
        ui_overlay_texture_->getBuffer()->blitFromMemory(ui);
    }

    void QOgreWorldView::ShowUiOverlay()
    {
        ui_overlay_->show();
        RenderOneFrame();
    }

    void QOgreWorldView::HideUiOverlay()
    {
        ui_overlay_->hide();
        RenderOneFrame();
    }
}
