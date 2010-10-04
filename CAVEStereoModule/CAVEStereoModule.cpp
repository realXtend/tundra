#include "StableHeaders.h"
#include "CAVEStereoModule.h"
#include "CAVEManager.h"
#include "StereoController.h"

#include "OgreRenderingModule.h"
namespace CAVEStereo
{

	std::string CAVEStereoModule::type_name_static_ = "CAVEStereo";

    CAVEStereoModule::CAVEStereoModule() :
        IModule(type_name_static_),
        stereo_(0),
        cave_(0)
    {

    }
    CAVEStereoModule::~CAVEStereoModule()
    {
        SAFE_DELETE(stereo_);
        SAFE_DELETE(cave_);
    }
    QVector<Ogre::RenderWindow*> CAVEStereoModule::GetCAVERenderWindows()
    {
        return cave_->getExternalWindows();
    }

    void CAVEStereoModule::Load()
    {

    }
    void CAVEStereoModule::PreInitialize()
    {

    }
    void CAVEStereoModule::Initialize()
    {

    }
    void CAVEStereoModule::PostInitialize()
    {
        OgreRenderer::OgreRenderingModule *rendererModule = framework_->GetModuleManager()->GetModule<OgreRenderer::OgreRenderingModule>().lock().get();
        if(rendererModule)
        {
            OgreRenderer::RendererPtr renderer = rendererModule->GetRenderer();
            if(renderer.get())
            {
                stereo_ = new StereoController(renderer.get(),this);
                cave_ = new CAVEManager(renderer.get());
                stereo_->InitializeUi();
                cave_->InitializeUi();
            }
        }
    }
    void CAVEStereoModule::Uninitialize()
    {

    }
    void CAVEStereoModule::Update(f64 frametime)
    {

    }
}
//! --- POCO Manifest Stuff ---
extern "C" void POCO_LIBRARY_API SetProfiler(Foundation::Profiler *profiler);
void SetProfiler(Foundation::Profiler *profiler)
{
    Foundation::ProfilerSection::SetProfiler(profiler);
}

using namespace CAVEStereo;

POCO_BEGIN_MANIFEST(IModule)
   POCO_EXPORT_CLASS(CAVEStereoModule)
POCO_END_MANIFEST
