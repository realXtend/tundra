// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_StereoManager_h
#define incl_OgreRenderingModule_StereoManager_h

#include "Foundation.h"
#include <QMap>


namespace OgreRenderer
{
    class Renderer;
    class StereoWidget;
    class StereoManager: public QObject 
    {
        Q_OBJECT
        public:
            StereoManager(Renderer* r);
            virtual ~StereoManager();

            
        public slots:
            void InitializeUi();


        private:

            Renderer* renderer_;
            StereoWidget* settings_widget_;
            
    };
}

#endif