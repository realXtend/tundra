#include "StableHeaders.h"
#include "NaaliRenderWindow.h"

#include <QWidget>
#include <QImage>

#include <utility>

using namespace std;

namespace
{
    const char rttTextureName[] = "MainWindow RTT";
    const char rttMaterialName[] = "MainWindow Material";
}

NaaliRenderWindow::NaaliRenderWindow()
:renderWindow(0),
overlay(0),
overlayContainer(0)
{
}

void NaaliRenderWindow::CreateRenderWindow(QWidget *targetWindow, const QString &name, int width, int height, int left, int top, bool fullscreen)
{
    Ogre::NameValuePairList params;

#ifdef WIN32
    params["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned int)targetWindow->winId());
#elif Q_WS_MAC
// qt docs say it's a HIViewRef on carbon,
// carbon docs say HIViewGetWindow gets a WindowRef out of it

#if 0
HIViewRef vref = (HIViewRef) nativewin-> winId ();
WindowRef wref = HIViewGetWindow(vref);
    winhandle = Ogre::StringConverter::toString(
       (unsigned long) (HIViewGetRoot(wref)));
#else
    // according to
    // http://www.ogre3d.org/forums/viewtopic.php?f=2&t=27027 does
    winhandle = Ogre::StringConverter::toString(
                 (unsigned long) nativewin->winId());
#endif
    //Add the external window handle parameters to the existing params set.
    params["externalWindowHandle"] = winhandle;
#endif

#ifdef Q_WS_X11
    // GLX - According to Ogre Docs:
    // poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*)
    QX11Info info =  x11Info ();

    Ogre::String winhandle  = Ogre::StringConverter::toString 
        ((unsigned long)
         (info.display ()));
    winhandle += ":";

    winhandle += Ogre::StringConverter::toString 
        ((unsigned int)
         (info.screen ()));
    winhandle += ":";
    
    winhandle += Ogre::StringConverter::toString 
        ((unsigned long)
         nativewin-> winId());

    //Add the external window handle parameters to the existing params set.
    params["parentWindowHandle"] = winhandle;
#endif

    // Window position to params
    if (left != -1)
        params["left"] = ToString(left);
    if (top != -1)
        params["top"] = ToString(top);

#ifdef USE_NVIDIA_PERFHUD
    params["useNVPerfHUD"] = "true";
    params["Rendering Device"] = "NVIDIA PerfHUD";
#endif

    renderWindow = Ogre::Root::getSingletonPtr()->createRenderWindow(name.toStdString().c_str(), width, height, fullscreen, &params);
    renderWindow->setDeactivateOnFocusChange(false);

    CreateRenderTargetOverlay(width, height);
}

void NaaliRenderWindow::CreateRenderTargetOverlay(int width, int height)
{
    width = max(1, width);
    height = max(1, height);

    Ogre::TexturePtr renderTarget = Ogre::TextureManager::getSingleton().createManual(
        rttTextureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, width, height, 0, Ogre::PF_A8R8G8B8, Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE);

    Ogre::MaterialPtr rttMaterial = Ogre::MaterialManager::getSingleton().create(
        rttMaterialName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    Ogre::TextureUnitState *rttTuState = rttMaterial->getTechnique(0)->getPass(0)->createTextureUnitState();

    rttTuState->setTextureName(rttTextureName);
    rttTuState->setTextureFiltering(Ogre::TFO_NONE);
    rttTuState->setNumMipmaps(1);
    rttTuState->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);

    rttMaterial->setFog(true, Ogre::FOG_NONE); ///\todo Check, shouldn't here be false?
    rttMaterial->setReceiveShadows(false);
    rttMaterial->setTransparencyCastsShadows(false);

    rttMaterial->getTechnique(0)->getPass(0)->setSceneBlending(Ogre::SBF_SOURCE_ALPHA, Ogre::SBF_ONE_MINUS_SOURCE_ALPHA);
    rttMaterial->getTechnique(0)->getPass(0)->setDepthWriteEnabled(false);
    rttMaterial->getTechnique(0)->getPass(0)->setDepthCheckEnabled(false);
    rttMaterial->getTechnique(0)->getPass(0)->setLightingEnabled(false);
    rttMaterial->getTechnique(0)->getPass(0)->setCullingMode(Ogre::CULL_NONE);

    overlayContainer = Ogre::OverlayManager::getSingleton().createOverlayElement("Panel", "MainWindow Overlay Panel");
    overlayContainer->setMaterialName(rttMaterialName);
    overlayContainer->setMetricsMode(Ogre::GMM_PIXELS);
    overlayContainer->setPosition(0, 0);

    overlay = Ogre::OverlayManager::getSingleton().create("MainWindow Overlay");
    overlay->add2D(static_cast<Ogre::OverlayContainer *>(overlayContainer));
    overlay->setZOrder(500);
    overlay->show();

//    ResizeOverlay(width, height);
}

Ogre::RenderWindow *NaaliRenderWindow::OgreRenderWindow()
{
    return renderWindow;
}

Ogre::Overlay *NaaliRenderWindow::OgreOverlay()
{
    return overlay;
}

#if 0

void NaaliRenderWindow::RenderFrame()
{
    PROFILE(NaaliRenderWindow_RenderFrame);
/*
    if (applyFPSLimit && targetFpsLimit > 0.f)
    {
        Core::tick_t timeNow = Core::GetCurrentClockTime();

        double msecsSpentInFrame = (double)(timeNow - lastPresentTime) / timerFrequency * 1000.0;
        const double msecsPerFrame = 1000.0 / targetFpsLimit;
        if (msecsSpentInFrame < msecsPerFrame)
        {
            PROFILE(FPSLimitSleep);
            while(msecsSpentInFrame >= 0 && msecsSpentInFrame < msecsPerFrame)
            {
                if (msecsSpentInFrame + 1 < msecsPerFrame)
                    Sleep(1);

                msecsSpentInFrame = (double)(Core::GetCurrentClockTime() - lastPresentTime) / timerFrequency * 1000.0;
            }

            // Busy-wait the rest of the time slice to avoid jittering and to produce smoother updates.
            while(msecsSpentInFrame >= 0 && msecsSpentInFrame < msecsPerFrame)
                msecsSpentInFrame = (double)(Core::GetCurrentClockTime() - lastPresentTime) / timerFrequency * 1000.0;
        }

        lastPresentTime = Core::GetCurrentClockTime();

        timeSleptLastFrame = (float)((timeNow - lastPresentTime) * 1000.0 / timerFrequency);
    }
    else
        timeSleptLastFrame = 0.f;
*/
}
#endif

void NaaliRenderWindow::UpdateOverlayImage(const QImage &src)
{
    PROFILE(NaaliRenderWindow_UpdateOverlayImage);

    Ogre::Box bounds(0, 0, src.width(), src.height());
    Ogre::PixelBox bufbox(bounds, Ogre::PF_A8R8G8B8, (void *)src.bits());

    Ogre::TextureManager &mgr = Ogre::TextureManager::getSingleton();
    Ogre::TexturePtr texture = mgr.getByName(rttTextureName);
    assert(texture.get());
    texture->getBuffer()->blitFromMemory(bufbox);
}

void NaaliRenderWindow::Resize(int width, int height)
{
    renderWindow->resize(width, height);
    renderWindow->windowMovedOrResized();

    if (Ogre::TextureManager::getSingletonPtr() && Ogre::OverlayManager::getSingletonPtr())
    {
        PROFILE(NaaliRenderWindow_Resize);

        // recenter the overlay
//        int left = (renderWindow->getWidth() - width) / 2;
//        int top = (renderWindow->getHeight() - height) / 2;

        // resize the container
 //       overlayContainer->setDimensions(width, height);
  //      overlayContainer->setPosition(left, top);
        overlayContainer->setDimensions(width, height);
        overlayContainer->setPosition(0,0);

        // resize the backing texture
        Ogre::TextureManager &mgr = Ogre::TextureManager::getSingleton();
        Ogre::TexturePtr texture = mgr.getByName(rttTextureName);
        assert(texture.get());

        texture->freeInternalResources();
        texture->setWidth(width);
        texture->setHeight(height);
        texture->createInternalResources();
    }
}