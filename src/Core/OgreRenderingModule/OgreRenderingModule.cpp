// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "EC_Placeable.h"
#include "EC_Mesh.h"
#include "EC_OgreCustomObject.h"
#include "EC_AnimationController.h"
#include "EC_Camera.h"
#include "EC_Light.h"
#include "EC_OgreCompositor.h"
#include "EC_RttTarget.h"
#include "EC_Material.h"
#include "EC_Billboard.h"
#include "EC_ParticleSystem.h"
#include "EC_Fog.h"
#include "EC_EnvironmentLight.h"
#include "EC_Sky.h"
#include "EC_OgreShadowSetup.h"
#include "OgreWorld.h"
#include "OgreMeshAsset.h"
#include "OgreParticleAsset.h"
#include "OgreSkeletonAsset.h"
#include "OgreMaterialAsset.h"
#include "TextureAsset.h"

#include "Application.h"
#include "Entity.h"
#include "Scene/Scene.h"
#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "Profiler.h"
#include "ConsoleAPI.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"

#include "StaticPluginRegistry.h"

#include <OgreProfiler.h>
#ifdef OGRE_HAS_PROFILER_HOOKS
#include <OgreProfilerHook.h>
#endif

#include "MemoryLeakCheck.h"

namespace OgreRenderer
{

std::string OgreRenderingModule::CACHE_RESOURCE_GROUP = "TundraAssetCache";

#ifdef OGRE_HAS_PROFILER_HOOKS

#ifdef WIN32
DWORD mainThreadId;
#endif

void Profiler_BeginBlock(const char *name)
{
#if defined(PROFILING) && defined(WIN32)
    if (GetCurrentThreadId() != mainThreadId)
        return;
    Framework *fw = Framework::Instance();
    Profiler *p = fw ? fw->GetProfiler() : 0;
    if (p)
        p->StartBlock((std::string("OGRE_") + name).c_str());
#endif
}

void Profiler_EndBlock()
{
#if defined(PROFILING) && defined(WIN32)
    if (GetCurrentThreadId() != mainThreadId)
        return;
    Framework *fw = Framework::Instance();
    Profiler *p = fw ? fw->GetProfiler() : 0;
    if (p)
    {
        ProfilerNodeTree *treeNode = p->CurrentNode();
        if (treeNode)
            p->EndBlock(treeNode->Name());
    }
#endif
}

#endif

OgreRenderingModule::OgreRenderingModule() :
    IModule("OgreRendering"),
    renderSystemListener(0)
{
#ifdef OGRE_HAS_PROFILER_HOOKS
#ifdef WIN32
    mainThreadId = GetCurrentThreadId();
#endif
    OgreProfiler_BeginBlock = Profiler_BeginBlock;
    OgreProfiler_EndBlock = Profiler_EndBlock;
#endif
}

OgreRenderingModule::~OgreRenderingModule()
{
}

void OgreRenderingModule::Load()
{
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Placeable>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Mesh>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Light>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_OgreCustomObject>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_AnimationController>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Camera>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_OgreCompositor>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_RttTarget>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Material>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Billboard>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_ParticleSystem>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Fog>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_Sky>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_EnvironmentLight>));
    framework_->Scene()->RegisterComponentFactory(MAKE_SHARED(GenericComponentFactory<EC_OgreShadowSetup>));

    // Main ogre .mesh extension
    QStringList meshExtensions;
    meshExtensions << ".mesh";
    
    // These file types are supported by the Open Asset Import library:
    // http://assimp.sourceforge.net/main_features_formats.html
#ifdef ASSIMP_ENABLED
    meshExtensions << ".3d" << ".b3d" << ".dae" << ".bvh" << ".3ds" << ".ase" << ".obj" << ".ply" << ".dxf" 
        << ".nff" << ".smd" << ".vta" << ".mdl" << ".md2" << ".md3" << ".mdc" << ".md5mesh" << ".x" << ".q3o" 
        << ".q3s" << ".raw" << ".ac" << ".stl" << ".irrmesh" << ".irr" << ".off" << ".ter" << ".mdl" << ".hmp"
        << ".ms3d" << ".lwo" << ".lws" << ".lxo" << ".csm" << ".ply" << ".cob" << ".scn";
#endif

    // The following file types are from FreeImage's list of supported formats:
    // http://freeimage.sourceforge.net/features.html
    // The GIMP xcf is not in the list of known image formats, but detect it anyways.
    // Put the most common formats to the front as this list will be iterated to find asset types.
    QStringList textureExtensions;
    textureExtensions << ".dds" << ".png" << ".jpeg" << ".jpg" << ".gif" << ".bmp" << ".tga" << ".targa" << ".tiff" << ".tif"
        << ".cut" << ".exr" << ".g3"  << ".hdr" << ".ico" << ".iff" << ".j2k" << ".j2c" << ".jp2" << ".jif" << ".jpe" << ".jng" 
        << ".koa" << ".lbm" << ".mng" << ".pbm" << ".pcd" << ".pcx" << ".pfm" << ".pict" << ".psd" << ".pgm" <<  ".ppm" << ".ras" 
        << ".raw" << ".sgi" << ".wap" << ".wbmp" << ".wbm" << ".xbm" << ".xcf" << ".xpm";

    // Add CRN format that we handle in Tundra, decompressing it to DDS before passing the data to Ogre.
    textureExtensions << ".crn";

    // Create asset type factories for each asset OgreRenderingModule provides to the system.
    framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(GenericAssetFactory<OgreMeshAsset>, "OgreMesh", meshExtensions));

    // Loading materials crashes Ogre in headless mode because we don't have Ogre Renderer running, so only register the Ogre material asset type if not in headless mode.
    if (!framework_->IsHeadless())
    {
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(GenericAssetFactory<OgreMaterialAsset>, "OgreMaterial", ".material"));
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(GenericAssetFactory<TextureAsset>, "Texture", textureExtensions));
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(GenericAssetFactory<OgreParticleAsset>, "OgreParticle", ".particle"));
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(GenericAssetFactory<OgreSkeletonAsset>, "OgreSkeleton", ".skeleton"));
    }
    else
    {
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(NullAssetFactory, "OgreMaterial", ".material"));
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(NullAssetFactory, "Texture", textureExtensions));
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(NullAssetFactory, "OgreParticle", ".particle"));
        framework_->Asset()->RegisterAssetTypeFactory(MAKE_SHARED(NullAssetFactory, "OgreSkeleton", ".skeleton"));
    }

    if (framework_->HasCommandLineParameter("--hide_benign_ogre_messages"))
        LogWarning("--hide_benign_ogre_messages: this format of the command-line parameter is deprecated and support for it will be removed. Use --hideBenignOgreMessages instead.");
}

void OgreRenderingModule::Initialize()
{
    renderer = MAKE_SHARED(OgreRenderer::Renderer, framework_);
    assert(renderer);
    assert(!renderer->IsInitialized());

    // Initializing the Renderer crashes inside Ogre if the current working directory is not the same as the directory where Ogre plugins reside in.
    // So, temporarily set the working dir to the installation directory, and restore it after succeeding to load the plugins.
    QString cwd = Application::CurrentWorkingDirectory();
    Application::SetCurrentWorkingDirectory(Application::InstallationDirectory());

    framework_->App()->SetSplashMessage("Initializing Ogre");
    renderer->Initialize();

    // Restore the original cwd to not disturb the environment we are running in.
    Application::SetCurrentWorkingDirectory(cwd);

    // Register renderer.
    framework_->RegisterRenderer(renderer.get());
    framework_->RegisterDynamicObject("renderer", renderer.get());

    // Connect to scene change signals.
    connect(framework_->Scene(), SIGNAL(SceneCreated(Scene *, AttributeChange::Type)), SLOT(CreateOgreWorld(Scene *)));
    connect(framework_->Scene(), SIGNAL(SceneAboutToBeRemoved(Scene *, AttributeChange::Type)), SLOT(RemoveOgreWorld(Scene *)));

    framework_->Console()->RegisterCommand("renderStats", "Prints out render statistics.",
        this, SLOT(ConsoleStats()));
#if OGRE_PROFILING == 1
    framework_->Console()->RegisterCommand("ogreProf", "Toggles visibility of the Ogre profiler overlay.",
        this, SLOT(ToggleOgreProfilerOverlay()));
#endif
    framework_->Console()->RegisterCommand("setMaterialAttribute", "Sets an attribute on a material asset",
        this, SLOT(SetMaterialAttribute(const QStringList &)));

    // Create and add Ogre rendersystem listener.
    if (!framework_->IsHeadless() && Ogre::Root::getSingleton().getRenderSystem())
    {
        renderSystemListener = new OgreRenderSystemListener(this);
        Ogre::Root::getSingleton().getRenderSystem()->addListener(renderSystemListener);
    }
}

void OgreRenderingModule::Uninitialize()
{
    // We're shutting down. Force a release of all loaded asset objects from the Asset API so that 
    // no refs to Ogre assets remain - below 'renderer.reset()' is going to delete Ogre::Root.
    framework_->Asset()->ForgetAllAssets();

    // Clear up the renderer object, so that it will not be left dangling.
    framework_->RegisterRenderer(0);

    // Destroy Ogre rendersystem listener.
    if (renderSystemListener && Ogre::Root::getSingleton().getRenderSystem())
        Ogre::Root::getSingleton().getRenderSystem()->removeListener(renderSystemListener);
    SAFE_DELETE(renderSystemListener);
}

void OgreRenderingModule::ConsoleStats()
{
    if (framework_->IsHeadless())
        return;
    if (renderer)
    {
        const Ogre::RenderTarget::FrameStats& stats = renderer->GetCurrentRenderWindow()->getStatistics();
        ConsoleAPI *c = framework_->Console();
        c->Print("Average FPS: " + QString::number(stats.avgFPS));
        c->Print("Worst FPS: " + QString::number(stats.worstFPS));
        c->Print("Best FPS: " + QString::number(stats.bestFPS));
        c->Print("Triangles: " + QString::number(stats.triangleCount));
        c->Print("Batches: " + QString::number(stats.batchCount));
        return;
    }
    else
        LogError("No renderer found!");
}

void OgreRenderingModule::ToggleOgreProfilerOverlay()
{
#if OGRE_PROFILING == 1
    if (!framework_->IsHeadless())
    {
        bool enabled = Ogre::Profiler::getSingleton().getEnabled();
        Ogre::Profiler::getSingleton().setEnabled(!enabled);
    }
#else
    LogError("OgreRenderingModule::ToggleOgreProfilerOverlay: Ogre built without profiling support, cannot show profiler overlay.");
#endif
}

void OgreRenderingModule::CreateOgreWorld(Scene *scene)
{
    // Add an OgreWorld to the scene
    OgreWorldPtr newWorld = MAKE_SHARED(OgreWorld, renderer.get(), scene->shared_from_this());
    renderer->ogreWorlds[scene] = newWorld;
    scene->setProperty(OgreWorld::PropertyName(), QVariant::fromValue<QObject*>(newWorld.get()));
}

void OgreRenderingModule::RemoveOgreWorld(Scene *scene)
{
    OgreWorldPtr worldPtr = scene->Subsystem<OgreWorld>();
    if (worldPtr)
    {
        scene->setProperty(OgreWorld::PropertyName(), QVariant());
        renderer->ogreWorlds.erase(scene);
    }
}

void OgreRenderingModule::SetMaterialAttribute(const QStringList &params)
{
    if (params.size() < 3)
    {
        LogError("OgreRenderingModule::SetMaterialAttribute: Usage: SetMaterialAttribute(asset,attribute,value)");
        return;
    }
    AssetPtr assetPtr = framework_->Asset()->GetAsset(framework_->Asset()->ResolveAssetRef("", params[0]));
    if (!assetPtr || !assetPtr->IsLoaded())
    {
        LogError("OgreRenderingModule::SetMaterialAttribute: No asset found or not loaded");
        return;
    }
    OgreMaterialAssetPtr matAsset = dynamic_pointer_cast<OgreMaterialAsset>(assetPtr);
    if (!matAsset)
    {
        LogError("OgreRenderingModule::SetMaterialAttribute: Not a material asset");
        return;
    }
    matAsset->SetAttribute(params[1], params[2]);
}

// OgreRenderSystemListener

OgreRenderSystemListener::OgreRenderSystemListener(OgreRenderingModule* renderingModule) :
    renderingModule_(renderingModule)
{
}

OgreRenderSystemListener::~OgreRenderSystemListener()
{
}

void OgreRenderSystemListener::eventOccurred(const Ogre::String& eventName, const Ogre::NameValuePairList* /*parameters*/)
{
    if (!renderingModule_ || !renderingModule_->Renderer())
        return;
    if (eventName == "DeviceLost")
        renderingModule_->Renderer()->EmitDeviceLost();
    else if (eventName == "DeviceRestored") 
        renderingModule_->Renderer()->EmitDeviceRestored();
    else if(eventName == "DeviceCreated")
        renderingModule_->Renderer()->EmitDeviceCreated();
    else if(eventName == "DeviceReleased")
        renderingModule_->Renderer()->EmitDeviceReleased();
}

} // ~namespace OgreRenderer

extern "C"
{

#ifndef ANDROID
DLLEXPORT void TundraPluginMain(Framework *fw)
#else
DEFINE_STATIC_PLUGIN_MAIN(OgreRenderingModule)
#endif
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    fw->RegisterModule(new OgreRenderer::OgreRenderingModule());
}

}
