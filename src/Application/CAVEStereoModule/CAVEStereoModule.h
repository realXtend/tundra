// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "IModule.h"
#include "CAVEStereoModuleApi.h"

#include <QVector>
#include <QObject>

namespace Ogre { class RenderWindow; }

namespace CAVEStereo
{
    class StereoController;
    class CAVEManager;

    class CAVESTEREO_MODULE_API CAVEStereoModule : public IModule
    {
        Q_OBJECT

    public:
        /// Constructor.
        CAVEStereoModule();

        /// Deconstructor.
        ~CAVEStereoModule();

        /// IModule override.
        void Initialize();

        /// Get current Ogre renderer windows.
        QVector<Ogre::RenderWindow*> GetCAVERenderWindows();

    public slots:
        /// Show the stereoscopy widget.
        void ShowStereoscopyWindow();

        /// Show the cave widget.
        void ShowCaveWindow();
        void TakeScreenshots(QString path, QString filename);
        void EnableStereo(QString tech_type, qreal eye_dist, qreal focal_l, qreal offset, qreal scrn_width);

    private:
        /// Manager for stereo.
        StereoController* stereo_;

        /// Manager for CAVE.
        CAVEManager* cave_;
    };
}
