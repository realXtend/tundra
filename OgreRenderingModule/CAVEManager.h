// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_CaveManager_h
#define incl_OgreRenderingModule_CaveManager_h

#include "Foundation.h"
#include <QMap>

namespace Ogre
{
    class Vector3;
}
namespace OgreRenderer
{
    class Renderer;
    class CAVESettingsWidget;
    class CAVEView;

	//! This class handles the CAVE view managing
    class CAVEManager: public QObject 
    {
        Q_OBJECT
        public:
            CAVEManager(Renderer* r);
            virtual ~CAVEManager();

			//!Disable cave (will also remove all views)
            void DisableCAVE();
			//!Enable cave
            void EnableCAVE();
            
        public slots:
			//initializes ui
            void InitializeUi();

			//Slot to toggle cave on/off
            void CAVEToggled(bool val);
			//Add view, this version lets you to decide on window width/height
            void AddView(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);
			/*Add view
			@param name of the view (must be unique)
			@param top_left topleft corner of the view
			@param bottom_left bottom left corner of the view
			@param bottom_right bottom right corner of the view
			@param eye_pos position of the eye, relative to view
			*/
            void AddView(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

        private:
			//! is cave enabled
            bool enabled_;
			//! renderer handle
            Renderer* renderer_;
			//! views
            QMap<QString, CAVEView*> view_map_;
			//! settings widget
            CAVESettingsWidget* settings_widget_;
            
    };
}

#endif