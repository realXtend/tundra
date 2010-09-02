#include "StableHeaders.h"
#include "StereoManager.h"
#include "StereoWidget.h"
#include "Renderer.h"
namespace OgreRenderer
{

    StereoManager::StereoManager(Renderer* r)
        :renderer_(r),
        settings_widget_(0)
    {
        
    }

	StereoManager::~StereoManager()
	{

	}

    void StereoManager::InitializeUi()
    {
        settings_widget_ = new StereoWidget(renderer_->GetFramework());
       
    }
}