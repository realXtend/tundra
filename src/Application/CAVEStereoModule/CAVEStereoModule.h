// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CAVEStereoModule_CAVEStereoModule_h
#define incl_CAVEStereoModule_CAVEStereoModule_h

#include "IModule.h"
#include "CAVEStereoModuleApi.h"

#include <QVector>
#include <QObject>

namespace Ogre { class RenderWindow; }

namespace CAVEStereo
{
    class StereoController;
    class CAVEManager;

    class CAVESTEREO_MODULE_API CAVEStereoModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        /// Constructor.
        CAVEStereoModule();

        /// Deconstructor.
        ~CAVEStereoModule();

        /// IModule override.
        virtual void PostInitialize();

        /// Get current Ogre renderer windows.
        QVector<Ogre::RenderWindow*> GetCAVERenderWindows();

    public slots:
        /// Show the stereoscopy widget.
        void ShowStereoscopyWindow();

        /// Show the cave widget.
        void ShowCaveWindow();

    private:
        /// Manager for stereo.
        StereoController* stereo_;

        /// Manager for CAVE.
        CAVEManager* cave_;
    };
}

#endif
