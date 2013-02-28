// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "OgreModuleFwd.h"
#include "CAVEView.h"

#include <QObject>
#include <QPointer>
#include <QMap>

namespace Ogre { class Vector3; }

namespace CAVEStereo
{
    class CAVESettingsWidget;

    /// This class handles the CAVE view managing
    class CAVEManager: public QObject 
    {
        Q_OBJECT

    public:
        CAVEManager(const OgreRenderer::RendererPtr &renderer);
        virtual ~CAVEManager();

        ///Disable cave (will also remove all views)
        void DisableCAVE();

        ///Enable cave
        void EnableCAVE();

        /// get windows used by cave manager. NOTE: when cave is shutdown, the manager will destroy the windows
        QVector<Ogre::RenderWindow*> GetExternalWindows();
        
    public slots:
        //initializes ui
        void InitializeUi();

        //Slot to toggle cave on/off
        void CAVEToggled(bool val);

        //Add view, this version lets you to decide on window width/height
        void AddView(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

        void AddPanoramaView(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos,int n);

        /// Adds view
        /** @param name of the view (must be unique)
            @param top_left top left corner of the view
            @param bottom_left bottom left corner of the view
            @param bottom_right bottom right corner of the view
            @param eye_pos position of the eye, relative to view
        */
        void AddView(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

        void AddPanoramaView(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos,int windows_number);

        void RemoveView(const QString& name);

        void ModifyView(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos); 

        void GetViewParametersView(const QString& name, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos); 

        CAVEStereo::CAVESettingsWidget* GetCaveWidget() const;

    private:
        /// is cave enabled
        bool enabled_;

        /// renderer pointer
        OgreRenderer::RendererWeakPtr renderer_;

        /// views
        QMap<QString, QPointer<CAVEView> > view_map_;

        /// settings widget
        CAVESettingsWidget* settings_widget_;
    };
}
