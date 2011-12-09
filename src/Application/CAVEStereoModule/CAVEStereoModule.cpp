// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CoreDefines.h"
#include "OgreRenderingModule.h"
#include "Renderer.h"
#include "LoggingFunctions.h"

#include "CAVEStereoModule.h"
#include "CAVEManager.h"
#include "CAVESettingsWidget.h"
#include "StereoController.h"
#include "StereoWidget.h"

#include "MemoryLeakCheck.h"

namespace CAVEStereo
{
    CAVEStereoModule::CAVEStereoModule() :
        IModule("CAVEStereo"),
        stereo_(0),
        cave_(0)
    {
    }

    CAVEStereoModule::~CAVEStereoModule()
    {
        SAFE_DELETE(stereo_);
        SAFE_DELETE(cave_);
    }

    void CAVEStereoModule::Initialize()
    {
        // This module does not register any components or anything important for headless runs.
        // Purely visual/rendering code so return in headless mode.
        if (framework_->IsHeadless())
            return;

        OgreRenderer::OgreRenderingModule *renderingModule = framework_->GetModule<OgreRenderer::OgreRenderingModule>();
        if (!renderingModule)
        {
            LogError("CAVEStereoModule: Could not acquire OgreRenderingModule for Renderer!");
            return;
        }

        OgreRenderer::RendererPtr renderer = renderingModule->GetRenderer();
        if (renderer.get())
        {
            stereo_ = new StereoController(renderer.get(), this);
            cave_ = new CAVEManager(renderer);
            stereo_->InitializeUi();
            cave_->InitializeUi();
        }
        else
            LogError("CAVEStereoModule: Renderer is null on startup, what now!");
    }

    QVector<Ogre::RenderWindow*> CAVEStereoModule::GetCAVERenderWindows()
    {
        return cave_->GetExternalWindows();
    }

    void CAVEStereoModule::ShowStereoscopyWindow()
    {
        stereo_->GetStereoWidget()->show();
    }

    void CAVEStereoModule::ShowCaveWindow()
    {
        cave_->GetCaveWidget()->show();
    }

    void CAVEStereoModule::TakeScreenshots(QString path, QString filename) 
    {
        stereo_->TakeScreenshots(path, filename);
    }

    void CAVEStereoModule::EnableStereo(QString tech_type, qreal eye_dist, qreal focal_l, qreal offset, qreal scrn_width) 
    {
        stereo_->EnableStereo(tech_type, eye_dist, focal_l, offset, scrn_width);
    }
}

extern "C"
{
    DLLEXPORT void TundraPluginMain(Framework *fw)
    {
        Framework::SetInstance(fw); // Inside this DLL, remember the pointer to the global framework object.
        IModule *module = new CAVEStereo::CAVEStereoModule();
        fw->RegisterModule(module);
    }
}
