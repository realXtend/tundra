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
        :renderer_(r),
        settings_widget_(0)
    {
        
    }

    void CAVEManager::InitializeUi()
    {
        settings_widget_ = new CAVESettingsWidget(renderer_->GetFramework());
        QObject::connect(settings_widget_, SIGNAL(ToggleCAVE(bool)),this, SLOT(CAVEToggled(bool)) );
    }
    CAVEManager::~CAVEManager()
    {

    }

    void CAVEManager::CAVEToggled(bool val)
    {
        if(val)
        {
            ConstructVCAVE();
            EnableCAVE();
        }
        else
        {
            DisableCAVE();
        }
    }
    void CAVEManager::AddView(const QString& name, qreal window_width, qreal window_height, qreal leftangle, qreal rightangle, qreal topangle, qreal bottomangle, qreal yaw, qreal aspect_ratio)
    {
        CAVEView* view = new CAVEView(renderer_);
        //view->Initialize(name, window_width, window_height, leftangle, rightangle, topangle, bottomangle, yaw, aspect_ratio);
        view_map_[name] = view;
    }

    void CAVEManager::AddView(const QString& name)
    {

    }

    void CAVEManager::DisableCAVE()
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
    
    void CAVEManager::ConstructVCAVE()
    {
        {
        CAVEView* view = new CAVEView(renderer_);
        Ogre::Vector3 eye(0,2,0);
        Ogre::Vector3 top_left(0,2,-4);
        Ogre::Vector3 bottom_left(0,0,-4);
        Ogre::Vector3 bottom_right(4,0,-3);
        view->Initialize("Test1",top_left,bottom_left,bottom_right,eye);
        view_map_["Test1"] = view;
        }
        {
        CAVEView* view = new CAVEView(renderer_);
        Ogre::Vector3 eye(0,2,0);
        Ogre::Vector3 top_left(-4,2,-3);
        Ogre::Vector3 bottom_left(-4,0,-3);
        Ogre::Vector3 bottom_right(0,0,-4);
        view->Initialize("Test2",top_left,bottom_left,bottom_right,eye);
        view_map_["Test2"] = view;
        }
    }
}