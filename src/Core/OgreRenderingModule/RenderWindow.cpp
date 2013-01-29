// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "Profiler.h"

#include "RenderWindow.h"
#include "CoreStringUtils.h"

#include <QWidget>
#include <QImage>

#include <utility>

#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include <OgreOverlayManager.h>
#include <OgrePanelOverlayElement.h>

#ifdef ANDROID
#include <jni.h>
#include <android/log.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include "Framework.h"
#endif

#include "LoggingFunctions.h"

#include "MemoryLeakCheck.h"

#ifdef ANDROID
jclass qtApplicationClass;
jmethodID getSurfaceMethod;
#endif

using namespace std;

namespace
{
    const char rttTextureName[] = "MainWindow RTT";
    const char rttMaterialName[] = "MainWindow Material";
}

RenderWindow::RenderWindow()
:renderWindow(0),
overlay(0),
overlayContainer(0)
{
}

void RenderWindow::CreateRenderWindow(QWidget *targetWindow, const QString &name, int width, int height, int left, int top, bool fullscreen, Framework *fw)
{
    Ogre::NameValuePairList params;

#ifdef ANDROID
    fullscreen = true;

    /// \todo This should not be done here, but doing it in Tundra's main seems to be unreliable and lead to crashes
    JavaVM* vm = Framework::JavaVMInstance();
    JNIEnv* env;
    vm->AttachCurrentThread(&env, 0);
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
	LogError("CreateRenderWindow: GetEnv failed");
	return;
    }
    Framework::SetJniEnvInstance(env);

    if (!env)
    {    
	LogError("CreateRenderWindow: Null Java environment, can not call Java methods");
	return;
    }

    jobject result = env->CallStaticObjectMethod(qtApplicationClass, getSurfaceMethod);
    if (!result)
    {
        LogError("CreateRenderWindow: Surface was null");
	return;
    }  

/*
    jclass clsID = env->GetObjectClass(result);
    jmethodID msgMethodID;
    const char* localstr = NULL;
    jclass javaClassClsID = env->FindClass("java/lang/Class");
    msgMethodID = env->GetMethodID(javaClassClsID, "getName", "()Ljava/lang/String;");
    if(msgMethodID == NULL)
        LogInfo("Null getName method");
    else 
    {
        jstring clsName = (jstring)env->CallObjectMethod(clsID, msgMethodID);
        localstr = env->GetStringUTFChars(clsName, NULL);
        LogInfo("Surface class name: " + QString(localstr));
    }
*/

    LogInfo("CreateRenderWindow: calling ANativeWindow_fromSurface");
    ANativeWindow* window = ANativeWindow_fromSurface(env, result);

    params["externalWindowHandle"] = Ogre::StringConverter::toString((int)window);
#endif

    // See http://www.ogre3d.org/tikiwiki/RenderWindowParameters
    if (fw->CommandLineParameters("--vsync").length() > 0) // "Synchronize buffer swaps to monitor vsync, eliminating tearing at the expense of a fixed frame rate"
        params["vsync"] = ParseBool(fw->CommandLineParameters("--vsync").first());
    if (fw->CommandLineParameters("--vsyncFrequency").length() > 0) // "Display frequency rate; only applies if fullScreen is set."
        params["displayFrequency"] = fw->CommandLineParameters("--vsyncFrequency").first().toInt();
    if (fw->CommandLineParameters("--antialias").length() > 0) // "Full screen antialiasing factor"
        params["FSAA"] = fw->CommandLineParameters("--antialias").first().toInt();
#ifdef WIN32
    if (targetWindow)
        params["externalWindowHandle"] = Ogre::StringConverter::toString((unsigned int)targetWindow->winId());
#endif

#ifdef Q_WS_MAC
    Ogre::String winhandle;

    QWidget* nativewin = targetWindow;

    while(nativewin && nativewin->parentWidget())
        nativewin = nativewin->parentWidget();

    // according to
    // http://www.ogre3d.org/forums/viewtopic.php?f=2&t=27027 does
    winhandle = Ogre::StringConverter::toString(
        (unsigned long)nativewin ? nativewin->winId() : 0);

    //Add the external window handle parameters to the existing params set.
    params["externalWindowHandle"] = winhandle;
    
    /* According to http://doc.qt.nokia.com/stable/qwidget.html#winId
       "On Mac OS X, the type returned depends on which framework Qt was linked against. 
       -If Qt is using Carbon, the {WId} is actually an HIViewRef. 
       -If Qt is using Cocoa, {WId} is a pointer to an NSView."
      Ogre needs to know that a NSView handle will be passed to its' externalWindowHandle parameter,
      otherwise it assumes that NSWindow will be used 
    */
    params["macAPI"] = "cocoa";
    params["macAPICocoaUseNSView"] = "true";
#endif

#ifdef Q_WS_X11
    QWidget *parent = targetWindow;
    while(parent && parent->parentWidget())
        parent = parent->parentWidget();

    // GLX - According to Ogre Docs:
    // poslong:posint:poslong:poslong (display*:screen:windowHandle:XVisualInfo*)
    QX11Info info = targetWindow->x11Info();

    Ogre::String winhandle = Ogre::StringConverter::toString((unsigned long)(info.display()));
    winhandle += ":" + Ogre::StringConverter::toString((unsigned int)(info.screen()));
    winhandle += ":" + Ogre::StringConverter::toString((unsigned long)parent ? parent->winId() : 0);

    //Add the external window handle parameters to the existing params set.
    params["parentWindowHandle"] = winhandle;
#endif

    // Window position to params
    if (left != -1)
        params["left"] = Ogre::StringConverter::toString(left);
    if (top != -1)
        params["top"] = Ogre::StringConverter::toString(top);

#ifdef WIN32
    if (fw->HasCommandLineParameter("--perfHud"))
    {
        params["useNVPerfHUD"] = "true";
        params["Rendering Device"] = "NVIDIA PerfHUD";
    }
#endif

    renderWindow = Ogre::Root::getSingletonPtr()->createRenderWindow(name.toStdString().c_str(), width, height, fullscreen, &params);
    renderWindow->setDeactivateOnFocusChange(false);

    // Currently do not create UI overlay on Android to save fillrate
#ifndef ANDROID
    CreateRenderTargetOverlay(width, height);
#endif
}

void RenderWindow::CreateRenderTargetOverlay(int width, int height)
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
    overlayContainer->setDimensions((Ogre::Real)width, (Ogre::Real)height);
    overlayContainer->setPosition(0,0);

    overlay = Ogre::OverlayManager::getSingleton().create("MainWindow Overlay");
    overlay->add2D(static_cast<Ogre::OverlayContainer *>(overlayContainer));
    overlay->setZOrder(500);
    overlay->show();

//    ResizeOverlay(width, height);
}

Ogre::RenderWindow *RenderWindow::OgreRenderWindow() const
{
    return renderWindow;
}

Ogre::Overlay *RenderWindow::OgreOverlay() const
{
    return overlay;
}

std::string RenderWindow::OverlayTextureName() const
{
    return rttTextureName;
}

void RenderWindow::UpdateOverlayImage(const QImage &src)
{
    if (!overlay)
        return;

    PROFILE(RenderWindow_UpdateOverlayImage);

    Ogre::Box bounds(0, 0, src.width(), src.height());
    Ogre::PixelBox bufbox(bounds, Ogre::PF_A8R8G8B8, (void *)src.bits());

    Ogre::TextureManager &mgr = Ogre::TextureManager::getSingleton();
    Ogre::TexturePtr texture = mgr.getByName(rttTextureName);
    assert(texture.get());
    texture->getBuffer()->blitFromMemory(bufbox);
}

void RenderWindow::ShowOverlay(bool visible)
{
    if (overlayContainer)
    {
        if (!visible)
            overlayContainer->hide();
        else
            overlayContainer->show();
    }
}

void RenderWindow::Resize(int width, int height)
{
    assert(renderWindow);

    if (width == (int)renderWindow->getWidth() && height == (int)renderWindow->getHeight())
        return; // Avoid recreating resources if the size didn't actually change.

    renderWindow->resize(width, height);
    renderWindow->windowMovedOrResized();

    if (overlay && Ogre::TextureManager::getSingletonPtr() && Ogre::OverlayManager::getSingletonPtr())
    {
        PROFILE(RenderWindow_Resize);

        // recenter the overlay
//        int left = (renderWindow->getWidth() - width) / 2;
//        int top = (renderWindow->getHeight() - height) / 2;

        // resize the container
 //       overlayContainer->setDimensions(width, height);
  //      overlayContainer->setPosition(left, top);
        overlayContainer->setDimensions((Ogre::Real)width, (Ogre::Real)height);
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

    emit Resized(width, height);
}

int RenderWindow::Width() const
{
    return renderWindow->getWidth();
}

int RenderWindow::Height() const
{
    return renderWindow->getHeight();
}

#ifdef ANDROID
extern "C" {

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* /*reserved*/)
{
    __android_log_print(ANDROID_LOG_INFO,"Tundra", "Tundra JNI_OnLoad");
    Framework::SetJavaVMInstance(vm);

    JNIEnv *env = 0;
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        __android_log_print(ANDROID_LOG_FATAL,"Qt","GetEnv failed");
        return -1;
    }

    qtApplicationClass = env->FindClass("org/kde/necessitas/origo/QtApplication");
    if (!qtApplicationClass) 
    {
        __android_log_print(ANDROID_LOG_INFO,"Tundra", "Can not find QtApplication class");
        return -1;
    }
    qtApplicationClass = (jclass)env->NewGlobalRef(qtApplicationClass);

    getSurfaceMethod = env->GetStaticMethodID(qtApplicationClass, "getSurface", "()Ljava/lang/Object;");
    if (!getSurfaceMethod)
    {
        __android_log_print(ANDROID_LOG_INFO,"Tundra", "Can not find getSurface method from QtApplication class");
	return -1;
    }

    return JNI_VERSION_1_4;
}

}
#endif
