// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "Application.h"
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
#include "Entity.h"
#include "RendererSettings.h"
#include "OgreMeshAsset.h"
#include "OgreParticleAsset.h"
#include "OgreSkeletonAsset.h"
#include "OgreMaterialAsset.h"
#include "TextureAsset.h"

#include "AssetAPI.h"
#include "GenericAssetFactory.h"
#include "NullAssetFactory.h"
#include "ConsoleAPI.h"
#include "ConfigAPI.h"
#include "SceneAPI.h"
#include "IComponentFactory.h"
#include "Profiler.h"
#include "UiAPI.h"
#include "UiMainWindow.h"

#include "MemoryLeakCheck.h"

namespace OgreRenderer
{

OgreRenderingModule::OgreRenderingModule() : IModule("OgreRendering")
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
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<TextureAsset>("OgreTexture"))); // deprecated/old style.
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreParticleAsset>("OgreParticle")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new GenericAssetFactory<OgreSkeletonAsset>("OgreSkeleton")));
    }
    else
    {
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("OgreMaterial")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("Texture")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("OgreTexture"))); // deprecated/old style.
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("OgreParticle")));
        framework_->Asset()->RegisterAssetTypeFactory(AssetTypeFactoryPtr(new NullAssetFactory("OgreSkeleton")));
    }
}

void OgreRenderingModule::PreInitialize()
{
    std::string ogre_config_filename = Application::InstallationDirectory().toStdString() + "ogre.cfg"; ///\todo Unicode support!
#if defined (_WINDOWS) && (_DEBUG)
    std::string plugins_filename = "pluginsd.cfg";
#elif defined (_WINDOWS)
    std::string plugins_filename = "plugins.cfg";
#elif defined(__APPLE__)
    std::string plugins_filename = "plugins-mac.cfg";
#else
    std::string plugins_filename = "plugins-unix.cfg";
#endif

    plugins_filename = Application::InstallationDirectory().toStdString() + plugins_filename; ///\todo Unicode support!

    // Create renderer here, so it can be accessed in uninitialized state by other module's PreInitialize()
    std::string window_title = framework_->Config()->GetApplicationIdentifier().toStdString();

    renderer = OgreRenderer::RendererPtr(new OgreRenderer::Renderer(framework_, ogre_config_filename, plugins_filename, window_title));

}

void OgreRenderingModule::Initialize()
{
    assert (renderer);
    assert (!renderer->IsInitialized());

    // Initializing the Renderer crashes inside Ogre if the current working directory is not the same as the directory where Ogre plugins reside in.
    // So, temporarily set the working dir to the installation directory, and restore it after succeeding to load the plugins.
    QString cwd = Application::CurrentWorkingDirectory();
    Application::SetCurrentWorkingDirectory(Application::InstallationDirectory());

    renderer->Initialize();

    // Restore the original cwd to not disturb the enviroment we are running in.
    Application::SetCurrentWorkingDirectory(cwd);

    framework_->GetServiceManager()->RegisterService(Service::ST_Renderer, renderer);

    framework_->RegisterDynamicObject("renderer", renderer.get());
}

void OgreRenderingModule::PostInitialize()
{
    renderer->PostInitialize();

    framework_->Console()->RegisterCommand("RenderStats", "Prints out render statistics.",
        this, SLOT(ConsoleStats()));
    framework_->Console()->RegisterCommand("SetMaterialAttribute", "Sets an attribute on a material asset",
        this, SLOT(SetMaterialAttribute(const StringVector &)));
}

void OgreRenderingModule::Uninitialize()
{
    // We're shutting down. Force a release of all loaded asset objects from the Asset API so that 
    // no refs to Ogre assets remain - below 'renderer.reset()' is going to delete Ogre::Root.
    framework_->Asset()->ForgetAllAssets();

    framework_->GetServiceManager()->UnregisterService(renderer);

    // Explicitly remove log listener now, because the service has been
    // unregistered now, and no-one can get at the renderer to remove themselves
    if (renderer)
        renderer->RemoveLogListener();

    renderer.reset();
}

void OgreRenderingModule::Update(f64 frametime)
{
    {
        PROFILE(OgreRenderingModule_Update);
        renderer->Update(frametime);
    }
}

void OgreRenderingModule::ShowSettingsWindow()
{
    if (framework_->IsHeadless())
        return;

    if (settingsWindow)
    {
        settingsWindow->show();
        return;
    }

    settingsWindow = new RendererSettingsWindow(framework_, framework_->Ui()->MainWindow());
    settingsWindow->setWindowFlags(Qt::Tool);
    settingsWindow->setAttribute(Qt::WA_DeleteOnClose);
    settingsWindow->show();
}

void OgreRenderingModule::ConsoleStats()
{
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

void OgreRenderingModule::SetMaterialAttribute(const StringVector &params)
{
    if (params.size() < 3)
    {
        LogError("Usage: SetMaterialAttribute(asset,attribute,value)");
        return;
    }
    
    AssetAPI* asset = framework_->Asset();
    AssetPtr assetPtr = asset->GetAsset(asset->ResolveAssetRef("", QString::fromStdString(params[0])));
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
    matAsset->SetAttribute(QString::fromStdString(params[1]), QString::fromStdString(params[2]));
}

} // ~namespace OgreRenderer

using namespace OgreRenderer;

extern "C"
{
__declspec(dllexport) void TundraPluginMain(Framework *fw)
{
    Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
    IModule *module = new OgreRenderer::OgreRenderingModule();
    fw->GetModuleManager()->DeclareStaticModule(module);
}
}
