#include "StableHeaders.h"
#include "StereoController.h"
#include "StereoWidget.h"
#include "Renderer.h"
#include "StereoManager.h"
#include <QVector>
#include <QDebug>
#include "ExternalRenderWindow.h"
#include "OgreRenderingModule.h"





namespace OgreRenderer
{

    StereoController::StereoController(Renderer* r)
        :renderer_(r),
        settings_widget_(0),
		number_of_views_(0),
		prefix_("stereoview")
    {
        
    }

	StereoController::~StereoController()
	{
		
	}

	QVector<Ogre::RenderWindow*> StereoController::getRenderWindows()
	{
		QVector<Ogre::RenderWindow*> vec;
		vec.append(renderer_->GetCurrentRenderWindow());
		return vec;
	}

	void StereoController::ChangeShaderColour(qreal r, qreal g, qreal b,const Ogre::String& name)
	{
		try
		{
			Ogre::MaterialPtr mat = Ogre::MaterialManager::getSingleton().getByName("Stereo/Anaglyph");
			Ogre::Pass* pass = mat->getTechnique(0)->getPass(0);
			Ogre::GpuProgramParametersSharedPtr param = pass->getFragmentProgramParameters();
			Ogre::ColourValue colour(r,g,b,1);
			param->setNamedConstant(name, colour);
			
		}catch (Ogre::Exception &e)
        {
            OgreRenderingModule::LogWarning(e.what());
            OgreRenderingModule::LogWarning("Failed to set Anaglyph shader constant.");
		}
	}


	void StereoController::ChangeColorLeft(qreal r, qreal g, qreal b)
	{
		Ogre::String str("left_colour");
		ChangeShaderColour(r,g,b,str);
	}

	void StereoController::ChangeColorRight(qreal r, qreal g, qreal b)
	{
		Ogre::String str("right_colour");
		ChangeShaderColour(r,g,b,str);
	}

    void StereoController::InitializeUi()
    {
       settings_widget_ = new StereoWidget(renderer_->GetFramework());
	   QObject::connect(settings_widget_, SIGNAL(EnableStereo(QString&, qreal, qreal, qreal, qreal)), this,SLOT(EnableStereo(QString&, qreal, qreal, qreal, qreal) )); 
	   QObject::connect(settings_widget_, SIGNAL(DisableStereo()), this, SLOT(DisableStereo()));
	   QObject::connect(settings_widget_, SIGNAL(ChangeColorLeft(qreal, qreal, qreal)), this, SLOT(ChangeColorLeft(qreal, qreal, qreal)));
	   QObject::connect(settings_widget_, SIGNAL(ChangeColorRight(qreal, qreal, qreal)), this, SLOT(ChangeColorRight(qreal, qreal, qreal)));
    }

	void StereoController::EnableStereo(QString& tech_type, qreal eye_dist, qreal focal_l, qreal offset, qreal scrn_width)
	{
		DisableStereo();
		if(tech_type == "anaglyph")
		{
			QVector<Ogre::RenderWindow*> windows = getRenderWindows();
			for(int i=0; i< windows.size();i++)
			{
				StereoManager* mngr = new StereoManager();
				Ogre::RenderWindow *original_window = windows.at(i);
				Ogre::Viewport * viewport = original_window->getViewport(0);
				mngr->init(viewport,0,StereoManager::SM_ANAGLYPH);
				mngr->setEyesSpacing(eye_dist);
				mngr->setFocalLength(focal_l);
				mngr->setPixelOffset(offset);
				if(scrn_width > 0)
				{
					mngr->setScreenWidth(scrn_width);
				}
				
				QString name = prefix_;
				name += QString::number(number_of_views_);
				stereo_views_[name] = mngr;
				number_of_views_++;
			}
			
			
		}

		if(tech_type == "passive")
		{
			QVector<Ogre::RenderWindow*> windows = getRenderWindows();
			for(int i=0; i< windows.size();i++)
			{
				QString name = prefix_;
				name += QString::number(number_of_views_);
				StereoManager* mngr = new StereoManager();
				Ogre::RenderWindow *original_window = windows.at(i);
				Ogre::Viewport *viewport = original_window->getViewport(0);
				ExternalRenderWindow* window = new ExternalRenderWindow(); 
				window->CreateRenderWindow((name + "_right").toStdString(),original_window->getWidth(), original_window->getHeight(),0,0,false);
				window->setGeometry(20,20,original_window->getWidth(),original_window->getHeight());
				
				window->getRenderWindow()->addViewport(viewport->getCamera());
				windows_to_dispose_.append(window);
				mngr->init(viewport,window->getRenderWindow()->getViewport(0),StereoManager::SM_DUALOUTPUT);
				mngr->setEyesSpacing(eye_dist);
				mngr->setFocalLength(focal_l);
				mngr->setPixelOffset(offset);
				if(scrn_width > 0)
				{
					mngr->setScreenWidth(scrn_width);
				}
				/*else
				{
					mngr->setScreenWidth(original_window->getWidth());
				}*/
				
				
				stereo_views_[name] = mngr;
				number_of_views_++;
				window->show();
			}
		}
		if(tech_type == "active")
		{
			QVector<Ogre::RenderWindow*> windows = getRenderWindows();
			for(int i=0; i< windows.size();i++)
			{
				QString name = prefix_;
				name += QString::number(number_of_views_);
				StereoManager* mngr = new StereoManager();
				Ogre::RenderWindow *original_window = windows.at(i);
				Ogre::Viewport *viewport = original_window->getViewport(0);


				mngr->init(viewport,0,StereoManager::SM_SHUTTER);
				mngr->setEyesSpacing(eye_dist);
				mngr->setFocalLength(focal_l);
				mngr->setPixelOffset(offset);
				if(scrn_width > 0)
				{
					mngr->setScreenWidth(scrn_width);
				}
				/*else
				{
					mngr->setScreenWidth(original_window->getWidth());
				}*/
				
				
				stereo_views_[name] = mngr;
				number_of_views_++;
			}
		}
	}





	void StereoController::DisableStereo()
	{
		for(int i=0;i<stereo_views_.count();i++)
		{
			StereoManager* mngr = stereo_views_.values()[i];
			mngr->shutdown();
			delete mngr;
		}
		for(int i=0;i<windows_to_dispose_.count();i++)
		{
			ExternalRenderWindow* window = windows_to_dispose_.at(i);
			window->hide();
			renderer_->GetRoot()->detachRenderTarget(window->getRenderWindow());
			delete window;
		}

		windows_to_dispose_.clear();
		stereo_views_.clear();
	}



}