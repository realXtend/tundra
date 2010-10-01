#include "StableHeaders.h"
#include "CAVEManager.h"
#include <Ogre.h>
#include "CAVEView.h"
#include "Renderer.h"
#include "CAVESettingsWidget.h"
#include "ExternalRenderWindow.h"

#include <QDebug>

namespace CAVEStereo
{

    CAVEManager::CAVEManager(OgreRenderer::Renderer* r)
        :enabled_(false),
        renderer_(r),
        settings_widget_(0)
    {
        
    }

    void CAVEManager::InitializeUi()
    {
        settings_widget_ = new CAVESettingsWidget(renderer_->GetFramework());
        QObject::connect(settings_widget_, SIGNAL(ModifyCAVEViewProjParams(const QString&, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)), this, SLOT(ModifyView(const QString& , Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)));
        QObject::connect(settings_widget_, SIGNAL(RemoveCAVEView(const QString&)), this, SLOT(RemoveView(const QString&)));
        QObject::connect(settings_widget_, SIGNAL(GetCAVEViewProjParams(const QString&, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)), this, SLOT(GetViewParametersView(const QString&, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)), Qt::DirectConnection); 
        QObject::connect(settings_widget_, SIGNAL(ToggleCAVE(bool)),this, SLOT(CAVEToggled(bool)) );
        QObject::connect(settings_widget_, SIGNAL(NewCAVEViewRequested(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&)), this, SLOT(AddView(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&)));
    }
    CAVEManager::~CAVEManager()
    {
        DisableCAVE();
        if(!view_map_.empty())
        {
            foreach(CAVEView* view, view_map_.values())
            {
                delete view;
            }

        }
        view_map_.clear();

    }

    void CAVEManager::CAVEToggled(bool val)
    {
        enabled_=val;
        if(val)
        {
            EnableCAVE();
        }
        else
        {
            DisableCAVE();
        }
    }
    void CAVEManager::AddView(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        if(!view_map_.contains(name))
        {
            CAVEView* view = new CAVEView(renderer_);
            view->Initialize(name, window_width, window_height, top_left, bottom_left, bottom_right, eye_pos);
        
            view_map_[name] = view;
            if(enabled_)
                view->GetExternalRenderWindow()->show();
            else
                view->GetExternalRenderWindow()->hide();
        }
    }

    void CAVEManager::AddView(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        
        if(!view_map_.contains(name))
        {
            CAVEView* view = new CAVEView(renderer_);
            view->Initialize(name, top_left, bottom_left, bottom_right, eye_pos);
            view_map_[name] = view;
            if(enabled_)
                view->GetExternalRenderWindow()->show();
            else
                view->GetExternalRenderWindow()->hide();
        }

    }

    void CAVEManager::DisableCAVE()
    {
        if(!view_map_.empty())
        {
            foreach(CAVEView* view, view_map_.values())
            {
                view->GetExternalRenderWindow()->hide();
                
                
            }

        }
        
    }

    QVector<Ogre::RenderWindow*> CAVEManager::getExternalWindows()
    {
        QVector<Ogre::RenderWindow*> windows;
        foreach(CAVEView *view, view_map_.values())
        {
            windows.push_back(view->GetExternalRenderWindow()->getRenderWindow());
        }
        return windows;
    }

    void CAVEManager::EnableCAVE()
    {
        if(!view_map_.empty())
        {
            foreach(CAVEView* view, view_map_.values())
            {
                view->GetExternalRenderWindow()->show();
            }

        }
    }

    void CAVEManager::RemoveView(const QString& name)
    {
        if(!view_map_.empty())
        {
            CAVEView* view = view_map_.take(name);
            if(view)
            {
                view->GetExternalRenderWindow()->hide();
                delete view;
            }
        }
    }
    void CAVEManager::ModifyView(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        if(!view_map_.empty())
        {
            CAVEView* view = view_map_[name];
            if(view)
            {
                view->ReCalculateProjection(top_left, bottom_left, bottom_right, eye_pos);
            }
        }
    }

    void CAVEManager::GetViewParametersView(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        if(!view_map_.empty())
        {
            CAVEView* view = view_map_[name];
            if(view)
            {
                view->GetProjectionParameters(top_left, bottom_left, bottom_right, eye_pos);
            }
        }
    }
    
}