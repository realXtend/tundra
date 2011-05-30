// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ARModule.h"
#include "ARRenderer.h"

#include "Framework.h"
#include "IComponent.h"

#include "TundraLogicModule.h"
#include "Client.h"

#include "OgreRenderingModule.h"

#include "UiAPI.h"

#include "DevicesAPI.h"
#include "SceneAPI.h"
#include "SceneManager.h"

#include "CameraInputModule.h"
#include "CameraInput.h"

#include "EC_ARGeometry.h"

namespace AR
{
    const std::string AR::ARModule::moduleName_ = std::string("ARModule");

    ARModule::ARModule() :
        IModule(NameStatic()),
        isFeeding_(false),
        arRenderer_(0),
        renderer_(0),
        cameraInput_(0)
    {
        actionFeed_ = new QAction(QIcon("./data/ui/images/communications_iconVideo.png"), "Toggle Augmented Reality Feed", this);
        connect(actionFeed_, SIGNAL(triggered()), SLOT(OnFeedAction()));
    }

    ARModule::~ARModule()
    {
        SAFE_DELETE(arRenderer_);
    }

    void ARModule::Load()
    {
        DECLARE_MODULE_EC(EC_ARGeometry);
    }
    
    void ARModule::Initialize()
    {
        if (framework_->IsHeadless())
            return;

        // Connect to be signaled when scene has changed
        connect(framework_->Scene(), SIGNAL(DefaultWorldSceneChanged(Scene::SceneManager*)), this, SLOT(DefaultSceneAdded(Scene::SceneManager*)));

        // Get CameraInput object and connect to signals.
        CameraInputModule *cameraModule = framework_->GetModule<CameraInputModule>();
        cameraInput_ = cameraModule->GetCameraInput();
        if (cameraInput_)
        {
            connect(cameraInput_, SIGNAL(Capturing(bool)), SLOT(RealityCaptureStateChanged(bool)));
            connect(cameraInput_, SIGNAL(FrameUpdate(const QImage&)), SLOT(RealityFrameUpdate(const QImage&)));
        }
        else
            LogWarning("Could not acquire CameraInput ptr, reality feeding is disabled.");

        // Register the module to scripting languages
        framework_->RegisterDynamicObject("ar", this);
    }
    
    void ARModule::PostInitialize()
    {
        if (framework_->IsHeadless())
            return;

        // Connect to connected event for toolar action adding
        TundraLogic::TundraLogicModule *tundraLogic = framework_->GetModule<TundraLogic::TundraLogicModule>();
        if (tundraLogic)
        {
            TundraLogic::Client *client = tundraLogic->GetClient().get();
            if (client)
                connect(client, SIGNAL(Connected()), SLOT(ClientConnected()));
        }

        // Get ogre renderer
        boost::shared_ptr<OgreRenderer::OgreRenderingModule> renderingmodule = framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock();
        renderer_ = renderingmodule->GetRenderer().get();

        // Init AR Renderer
        arRenderer_ = new ARRenderer(renderer_);
    }

    void ARModule::Uninitialize()
    {
    }

    void ARModule::Update(f64 frametime)
    {
    }

    // Public

    bool ARModule::IsFeedingReality()
    {
        return isFeeding_;
    }

    void ARModule::StartRealityFeed()
    {
        if (!cameraInput_)
            return;
        if (cameraInput_->GetCaptureFps() < 30)
            cameraInput_->SetCaptureFps(30);
        cameraInput_->StartCapturing();
    }

    void ARModule::StopRealityFeed()
    {
        if (!cameraInput_)
            return;
        cameraInput_->StopCapturing();
    }

    // Private

    void ARModule::OnFeedAction()
    {
        if (IsFeedingReality())
            StopRealityFeed();
        else
            StartRealityFeed();
    }

    void ARModule::ClientConnected()
    {
        framework_->Ui()->EmitAddAction(actionFeed_);
    }

    void ARModule::RealityCaptureStateChanged(bool capturing)
    {
        isFeeding_ = capturing;
        emit FeedingReality(isFeeding_);
        arRenderer_->SetRealityFeedEnabled(isFeeding_);
    }

    void ARModule::RealityFrameUpdate(const QImage &frame)
    {
        arRenderer_->BlitRealityToTexture(frame);
    }

    void ARModule::DefaultSceneAdded(Scene::SceneManager* mngr)
    {
        connect(mngr, SIGNAL(AttributeChanged(IComponent*, IAttribute*, AttributeChange::Type)), this, SLOT(ComponentAttributeChanged(IComponent* , IAttribute* , AttributeChange::Type )));
    }

    void ARModule::ComponentAttributeChanged(IComponent* comp, IAttribute* attribute, AttributeChange::Type change)
    {
        if (comp->TypeName() != "EC_ARGeometry")
            return;

        EC_ARGeometry* geom = dynamic_cast<EC_ARGeometry*>(comp);
        if (!geom)
            return;
        Ogre::Entity* ogreEnt = geom->GetOgreEntity();
        if (!ogreEnt)
            return;

        if (geom->getARVisible())
            arRenderer_->AddEntityToVisibleQueue(ogreEnt);
        else
            arRenderer_->AddEntityToInvisibleQueue(ogreEnt);
    }
}

extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace AR;

POCO_BEGIN_MANIFEST(IModule)
    POCO_EXPORT_CLASS(ARModule)
POCO_END_MANIFEST 
