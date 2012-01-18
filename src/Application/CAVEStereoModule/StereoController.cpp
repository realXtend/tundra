// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "Renderer.h"
#include "LoggingFunctions.h"

#include "StereoController.h"
#include "StereoWidget.h"
#include "StereoManager.h"
#include "ExternalRenderWindow.h"
#include "CAVEStereoModule.h"
#include <OgreRenderTarget.h>
#include <OgreRoot.h>

#include <QVector>

namespace CAVEStereo
{
    StereoController::StereoController(OgreRenderer::Renderer* r, CAVEStereoModule* mod) :
        renderer_(r),
        module_(mod),
        settings_widget_(0),
        number_of_views_(0),
        prefix_("stereoview"),
        flip_(false)
    {
    }

    StereoController::~StereoController()
    {
        if (settings_widget_)
            settings_widget_->deleteLater();
    }

    QVector<Ogre::RenderWindow*> StereoController::getRenderWindows()
    {
        QVector<Ogre::RenderWindow*> vec;
        vec.append(renderer_->GetCurrentRenderWindow());
        vec += module_->GetCAVERenderWindows();
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
        }
        catch (Ogre::Exception &e)
        {
            LogWarning("StereoController: Failed to set Anaglyph shader constant.");
            LogWarning(e.what());
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
        QObject::connect(settings_widget_, SIGNAL(StereoFlip()), this, SLOT(StereoFlip()));
    }

    void StereoController::EnableStereo(QString& tech_type, qreal eye_dist, qreal focal_l, qreal offset, qreal scrn_width)
    {
        DisableStereo();
        if (tech_type == "anaglyph")
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
        else if (tech_type == "passive")
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
                    mngr->setScreenWidth(scrn_width);
                
                stereo_views_[name] = mngr;
                number_of_views_++;
                window->show();
            }
        }
        else if (tech_type == "active")
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
                    mngr->setScreenWidth(scrn_width);

                stereo_views_[name] = mngr;
                number_of_views_++;
            }
        }
        else if (tech_type == "horizontal")
        {
            QVector<Ogre::RenderWindow*> windows = getRenderWindows();
            for(int i=0; i< windows.size();i++)
            {
                QString name = prefix_;
                name += QString::number(number_of_views_);
                StereoManager* mngr = new StereoManager();
                Ogre::RenderWindow *original_window = windows.at(i);
                Ogre::Viewport *viewport = original_window->getViewport(0);


                mngr->init(viewport,0,StereoManager::SM_INTERLACED_H);
                mngr->setEyesSpacing(eye_dist);
                mngr->setFocalLength(focal_l);
                mngr->setPixelOffset(offset);
                mngr->inverseStereo(flip_);

                if(scrn_width > 0)
                    mngr->setScreenWidth(scrn_width);

                stereo_views_[name] = mngr;
                number_of_views_++;

                flip_ = mngr->isStereoInversed();
            }
        }
        else if (tech_type == "vertical")
        {
            QVector<Ogre::RenderWindow*> windows = getRenderWindows();
            for(int i=0; i< windows.size();i++)
            {
                QString name = prefix_;
                name += QString::number(number_of_views_);
                StereoManager* mngr = new StereoManager();
                Ogre::RenderWindow *original_window = windows.at(i);
                Ogre::Viewport *viewport = original_window->getViewport(0);


                mngr->init(viewport,0,StereoManager::SM_INTERLACED_V);
                mngr->setEyesSpacing(eye_dist);
                mngr->setFocalLength(focal_l);
                mngr->setPixelOffset(offset);
                mngr->inverseStereo(flip_);
                if(scrn_width > 0)
                    mngr->setScreenWidth(scrn_width);

                stereo_views_[name] = mngr;
                number_of_views_++;

                flip_ = mngr->isStereoInversed();
            }
        }
        else if (tech_type == "checkboard")
        {
            QVector<Ogre::RenderWindow*> windows = getRenderWindows();
            for(int i=0; i< windows.size();i++)
            {
                QString name = prefix_;
                name += QString::number(number_of_views_);
                StereoManager* mngr = new StereoManager();
                Ogre::RenderWindow *original_window = windows.at(i);
                Ogre::Viewport *viewport = original_window->getViewport(0);


                mngr->init(viewport,0,StereoManager::SM_INTERLACED_CB);
                mngr->setEyesSpacing(eye_dist);
                mngr->setFocalLength(focal_l);
                mngr->setPixelOffset(offset);
                mngr->inverseStereo(flip_);

                if(scrn_width > 0)
                    mngr->setScreenWidth(scrn_width);

                stereo_views_[name] = mngr;
                number_of_views_++;

                flip_ = mngr->isStereoInversed();
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
            renderer_->OgreRoot()->detachRenderTarget(window->getRenderWindow());
            delete window;
        }

        windows_to_dispose_.clear();
        stereo_views_.clear();
    }

    void StereoController::StereoFlip()
    {
        flip_ = !flip_;
    }

    StereoWidget * StereoController::GetStereoWidget() const
    {
        return settings_widget_;
    }

    void StereoController::TakeScreenshots(QString path, QString filename)
    {
      
      if (stereo_views_.count() == 0) 
      {
          LogError("StereoController: Cannot take stereographic screenshots. Stereo not enabled!");
          return;
      }

      // FIXME relies on that there is only one external window and
      // its the right side window for stereographical stuff
      QString leftname = filename.replace(".jpg", "_left.jpg");
      QString rightname = filename.replace("_left.jpg", "_right.jpg");

      Ogre::CompositorInstance* comp = stereo_views_["stereoview0"]->GetCompositor();

      if (!comp) 
      {
          LogError("StereoController: Wrong stereographic mode. No Compositor. Cannot take screenshots!");
          return;
      }

      comp->getRenderTarget("Stereo/Left")->writeContentsToFile(path.toStdString() + leftname.toStdString());
      comp->getRenderTarget("Stereo/Right")->writeContentsToFile(path.toStdString() + rightname.toStdString());
    }
}
