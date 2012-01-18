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
#include "EC_SelectionBox.h"
#include "EC_Material.h"
#include "OgreWorld.h"
#include "OgreMeshAsset.h"
#include "OgreParticleAsset.h"
#include "OgreSkeletonAsset.h"
#include "OgreMaterialAsset.h"
#include "TextureAsset.h"

#include "Application.h"
#include "Entity.h"
#include "Scene.h"
#include "AssetAPI.h"
#include "AssetCache.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "ConsoleAPI.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"

#include "MemoryLeakCheck.h"

namespace OgreRenderer
{

std::string OgreRenderingModule::CACHE_RESOURCE_GROUP = "CACHED_ASSETS_GROUP";

OgreRenderingModule::OgreRenderingModule() : 
    IModule("OgreRendering")
{
}

OgreRenderingModule::~OgreRenderingModule()
{
}

void OgreRenderingModule::Load()
{
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Placeable>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Mesh>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Light>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_OgreCustomObject>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_AnimationController>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Camera>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_OgreCompositor>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_RttTarget>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_SelectionBox>));
    framework_->Scene()->RegisterComponentFactory(ComponentFactoryPtr(new GenericComponentFactory<EC_Material>));

    // Create asset type factories for each asset OgreRenderingModule provides to the system.
    framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreMeshAsset>("OgreMesh")));

    // Loading materials crashes Ogre in headless mode because we don't have Ogre Renderer running, so only register the Ogre material asset type if not in headless mode.
    if (!framework_->IsHeadless())
    {
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreMaterialAsset>("OgreMaterial")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<TextureAsset>("Texture")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreParticleAsset>("OgreParticle")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreSkeletonAsset>("OgreSkeleton")));
    }
    else
    {
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("OgreMaterial")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("Texture")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("OgreParticle")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("OgreSkeleton")));
    }
}

void OgreRenderingModule::Initialize()
{
    std::string ogreConfigFilename = Application::InstallationDirectory().toStdString() + "ogre.cfg"; ///\todo Unicode support!
#if defined (_WINDOWS) && (_DEBUG)
    std::string pluginsFilename = "pluginsd.cfg";
#elif defined (_WINDOWS)
    std::string pluginsFilename = "plugins.cfg";
#elif defined(__APPLE__)
    std::string pluginsFilename = "plugins-mac.cfg";
#else
    std::string pluginsFilename = "plugins-unix.cfg";
#endif

    pluginsFilename = Application::InstallationDirectory().toStdString() + pluginsFilename; ///\todo Unicode support!

    std::string windowTitle = Application::FullIdentifier().toStdString();

    renderer = boost::make_shared<OgreRenderer::Renderer>(framework_, ogreConfigFilename, pluginsFilename, windowTitle);
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
    connect(framework_->Scene(), SIGNAL(SceneAdded(const QString&)), this, SLOT(OnSceneAdded(const QString&)));
    connect(framework_->Scene(), SIGNAL(SceneRemoved(const QString&)), this, SLOT(OnSceneRemoved(const QString&)));

    // Add asset cache directory as its own resource group to ogre to support threaded loading.
    if (GetFramework()->Asset()->GetAssetCache())
    {
        std::string cacheResourceDir = GetFramework()->Asset()->GetAssetCache()->CacheDirectory().toStdString();
        if (!Ogre::ResourceGroupManager::getSingleton().resourceLocationExists(cacheResourceDir, CACHE_RESOURCE_GROUP))
            Ogre::ResourceGroupManager::getSingleton().addResourceLocation(cacheResourceDir, "FileSystem", CACHE_RESOURCE_GROUP);
    }

    framework_->Console()->RegisterCommand("RenderStats", "Prints out render statistics.",
        this, SLOT(ConsoleStats()));
    framework_->Console()->RegisterCommand("SetMaterialAttribute", "Sets an attribute on a material asset",
        this, SLOT(SetMaterialAttribute(const QStringList &)));
}

void OgreRenderingModule::Uninitialize()
{
    // We're shutting down. Force a release of all loaded asset objects from the Asset API so that 
    // no refs to Ogre assets remain - below 'renderer.reset()' is going to delete Ogre::Root.
    framework_->Asset()->ForgetAllAssets();

    // Clear up the renderer object, so that it will not be left dangling.
    framework_->RegisterRenderer(0);
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

void OgreRenderingModule::OnSceneAdded(const QString& name)
{
    ScenePtr scene = GetFramework()->Scene()->GetScene(name);
    if (!scene)
    {
        LogError("Could not find created scene");
        return;
    }
    
    // Add an OgreWorld to the scene
    OgreWorldPtr newWorld = boost::make_shared<OgreWorld>(renderer.get(), scene);
    renderer->ogreWorlds[scene.get()] = newWorld;
    scene->setProperty(OgreWorld::PropertyName(), QVariant::fromValue<QObject*>(newWorld.get()));
}

void OgreRenderingModule::OnSceneRemoved(const QString& name)
{
    // Remove the OgreWorld from the scene
    ScenePtr scene = GetFramework()->Scene()->GetScene(name);
    if (!scene)
    {
        LogError("Could not find scene about to be removed");
        return;
    }
    
    OgreWorld* worldPtr = scene->GetWorld<OgreWorld>().get();
    if (worldPtr)
    {
        scene->setProperty(OgreWorld::PropertyName(), QVariant());
        renderer->ogreWorlds.erase(scene.get());
    }
}

void OgreRenderingModule::SetMaterialAttribute(const QStringList &params)
{
    if (params.size() < 3)
    {
        LogError("Usage: SetMaterialAttribute(asset,attribute,value)");
        return;
    }
    AssetPtr assetPtr = framework_->Asset()->GetAsset(framework_->Asset()->ResolveAssetRef("", params[0]));
    if (!assetPtr || !assetPtr->IsLoaded())
    {
        LogError("No asset found or not loaded");
        return;
    }
    OgreMaterialAsset* matAsset = dynamic_cast<OgreMaterialAsset*>(assetPtr.get());
    if (!matAsset)
    {
        LogError("Not a material asset");
        return;
    }
    matAsset->SetAttribute(params[1], params[2]);
}

} // ~namespace OgreRenderer

using namespace OgreRenderer;

extern "C"
{
DLLEXPORT void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new OgreRenderer::OgreRenderingModule();
    fw->RegisterModule(module);
}
}
