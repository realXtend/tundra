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
    class CAVEManager: public QObject 
    {
        Q_OBJECT
        public:
            CAVEManager(Renderer* r);
            virtual ~CAVEManager();

            void DisableCAVE();
            void EnableCAVE();
            
        public slots:
            void InitializeUi();
            void CAVEToggled(bool val);
            void AddView(const QString& name, qreal window_width, qreal window_height, Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);
            void AddView(const QString& name,  Ogre::Vector3 &top_left, Ogre::Vector3 &bottom_left, Ogre::Vector3 &bottom_right, Ogre::Vector3 &eye_pos);

        private:

            bool enabled_;
            Renderer* renderer_;
            QMap<QString, CAVEView*> view_map_;
            CAVESettingsWidget* settings_widget_;
            
    };
}

#endif