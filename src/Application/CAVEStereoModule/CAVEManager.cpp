// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "CAVEManager.h"
#include "CAVEView.h"
#include "CAVESettingsWidget.h"
#include "ExternalRenderWindow.h"

#include "Renderer.h"

#include <Ogre.h>

#include "MemoryLeakCheck.h"

namespace CAVEStereo
{
    CAVEManager::CAVEManager(const OgreRenderer::RendererPtr &renderer):
        enabled_(false),
        renderer_(renderer),
        settings_widget_(0)
    {
    }

    void CAVEManager::InitializeUi()
    {
        settings_widget_ = new CAVESettingsWidget(renderer_.lock()->GetFramework());
        connect(settings_widget_, SIGNAL(ModifyCAVEViewProjParams(const QString&, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)), this, SLOT(ModifyView(const QString& , Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)));
        connect(settings_widget_, SIGNAL(RemoveCAVEView(const QString&)), this, SLOT(RemoveView(const QString&)));
        connect(settings_widget_, SIGNAL(GetCAVEViewProjParams(const QString&, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)), this, SLOT(GetViewParametersView(const QString&, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &, Ogre::Vector3 &)), Qt::DirectConnection); 
        connect(settings_widget_, SIGNAL(ToggleCAVE(bool)),this, SLOT(CAVEToggled(bool)) );
        connect(settings_widget_, SIGNAL(NewCAVEViewRequested(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&)), this, SLOT(AddView(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&)));
        connect(settings_widget_, SIGNAL(NewCAVEPanoramaViewRequested(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, int)), this, SLOT(AddPanoramaView(const QString&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, Ogre::Vector3&, int)));
    }

    CAVEManager::~CAVEManager()
    {
        DisableCAVE();
        foreach(QPointer<CAVEView> viewPtr, view_map_.values())
        {
            if (viewPtr)
                viewPtr->deleteLater();
        }

        if (settings_widget_)
            settings_widget_->deleteLater();
    }

    void CAVEManager::CAVEToggled(bool enabled)
    {
        enabled_ = enabled;
        if (enabled_)
            EnableCAVE();
        else
            DisableCAVE();
    }

    void CAVEManager::AddView(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        if (view_map_.contains(name))
            return;

        QPointer<CAVEView> viewPtr = new CAVEView(renderer_.lock());
        if (!viewPtr)
            return;

        viewPtr->Initialize(name, window_width, window_height, top_left, bottom_left, bottom_right, eye_pos);
        view_map_[name] = viewPtr;
        if (enabled_)
            viewPtr->GetExternalRenderWindow()->show();
        else
            viewPtr->GetExternalRenderWindow()->hide();
    }

    void CAVEManager::AddView(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        if (view_map_.contains(name))
            return;

        QPointer<CAVEView> viewPtr = new CAVEView(renderer_.lock());
        if (!viewPtr)
            return;

        viewPtr->Initialize(name, top_left, bottom_left, bottom_right, eye_pos);
        view_map_[name] = viewPtr;
        if (enabled_)
            viewPtr->GetExternalRenderWindow()->show();
        else
            viewPtr->GetExternalRenderWindow()->hide();
    }

    void CAVEManager::AddPanoramaView(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos, int window_number)
    {
        if (view_map_.contains(name))
            return;
        
        QPointer<CAVEView> viewPtr = new CAVEView(renderer_.lock());
        if (!viewPtr)
            return;

        viewPtr->InitializePanorama(name, window_width, window_height, top_left, bottom_left, bottom_right, eye_pos,window_number);
        view_map_[name] = viewPtr;
        if (enabled_)
            viewPtr->GetExternalRenderWindow()->show();
        else
            viewPtr->GetExternalRenderWindow()->hide();
    }

    void CAVEManager::AddPanoramaView(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos,int window_number)
    {
        if (view_map_.contains(name))
            return;

        QPointer<CAVEView> viewPtr = new CAVEView(renderer_.lock());
        if (!viewPtr)
            return;

        viewPtr->InitializePanorama(name, top_left, bottom_left, bottom_right, eye_pos,window_number);
        view_map_[name] = viewPtr;
        if (enabled_)
            viewPtr->GetExternalRenderWindow()->show();
        else
            viewPtr->GetExternalRenderWindow()->hide();
    }

    void CAVEManager::DisableCAVE()
    {
        if (view_map_.empty())
            return;
        foreach(QPointer<CAVEView> viewPtr, view_map_.values())
        {
            if (!viewPtr)
                continue;
            viewPtr->GetExternalRenderWindow()->hide();
        }
    }

    QVector<Ogre::RenderWindow*> CAVEManager::GetExternalWindows()
    {
        QVector<Ogre::RenderWindow*> windows;
        foreach(QPointer<CAVEView> viewPtr, view_map_.values())
        {
            if (!viewPtr)
                continue;
            windows.push_back(viewPtr->GetExternalRenderWindow()->getRenderWindow());
        }
        return windows;
    }

    void CAVEManager::EnableCAVE()
    {
        if(view_map_.empty())
            return;

        foreach(QPointer<CAVEView> viewPtr, view_map_.values())
        {
            if (!viewPtr)
                continue;
            viewPtr->GetExternalRenderWindow()->show();
        }
    }

    void CAVEManager::RemoveView(const QString& name)
    {
        if(!view_map_.empty())
        {
            QPointer<CAVEView> viewPtr = view_map_.take(name);
            if (viewPtr)
            {
                viewPtr->GetExternalRenderWindow()->close();
                viewPtr->deleteLater();
            }
        }
    }

    void CAVEManager::ModifyView(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        if(view_map_.empty())
            return;
        QPointer<CAVEView> viewPtr = view_map_[name];
        if (viewPtr)
            viewPtr->ReCalculateProjection(top_left, bottom_left, bottom_right, eye_pos);
    }

    void CAVEManager::GetViewParametersView(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos)
    {
        if (view_map_.empty())
            return;
        QPointer<CAVEView> viewPtr = view_map_[name];
        if (viewPtr)
            viewPtr->GetProjectionParameters(top_left, bottom_left, bottom_right, eye_pos);
    }

    CAVEStereo::CAVESettingsWidget* CAVEManager::GetCaveWidget() const
    {
        return settings_widget_;
    }
}
