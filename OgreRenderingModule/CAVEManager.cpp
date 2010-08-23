#include "StableHeaders.h"
#include "CAVEManager.h"
#include <Ogre.h>
#include "CAVEView.h"
#include "Renderer.h"
#include "CAVESettingsWidget.h"

#include <QDebug>

namespace OgreRenderer
{

    CAVEManager::CAVEManager(Renderer* r)
        :enabled_(false),
        renderer_(r),
        settings_widget_(0)
    {
        
    }

    void CAVEManager::InitializeUi()
    {
        settings_widget_ = new CAVESettingsWidget(renderer_->GetFramework());
        QObject::connect(settings_widget_, SIGNAL(ToggleCAVE(bool)),this, SLOT(CAVEToggled(bool)) );
        QObject::connect(settings_widget_, SIGNAL(NewCAVEViewRequested(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&)), this, SLOT(AddView(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&)));
    }
    CAVEManager::~CAVEManager()
    {
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
                view->show();
            else
                view->hide();
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
                view->show();
            else
                view->hide();
        }

    }

    void CAVEManager::DisableCAVE()
    {
        if(!view_map_.empty())
        {
            foreach(CAVEView* view, view_map_.values())
            {
                view->hide();
                
            }

        }
    }

    void CAVEManager::EnableCAVE()
    {
        if(!view_map_.empty())
        {
            foreach(CAVEView* view, view_map_.values())
            {
                view->show();
            }

        }
    }
    
}