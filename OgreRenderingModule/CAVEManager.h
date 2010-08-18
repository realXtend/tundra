// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_CaveManager_h
#define incl_OgreRenderingModule_CaveManager_h

#include "Foundation.h"
#include <QMap>


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
            
            void ConstructVCAVE();

            void DisableCAVE();
            void EnableCAVE();
            
        public slots:
            void InitializeUi();
            void CAVEToggled(bool val);

        private:
            void AddView(const QString& name, qreal window_width, qreal window_height, qreal leftangle, qreal rightangle, qreal topangle, qreal bottomangle, qreal yaw, qreal aspect_ratio);
            void AddView(const QString& name);
            Renderer* renderer_;
            QMap<QString, CAVEView*> view_map_;
            CAVESettingsWidget* settings_widget_;
            
    };
}

#endif