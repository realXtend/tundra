// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CAVEStereoModule_CAVEStereoModule_h
#define incl_CAVEStereoModule_CAVEStereoModule_h

#include "IModule.h"

#include "CAVEStereoModuleApi.h"
#include <QVector>
#include <QObject>

namespace Ogre
{
    class RenderWindow;
}

namespace Foundation
{
    class Framework;
}

namespace CAVEStereo
{
    class StereoController;
    class CAVEManager;

    class CAVESTEREO_MODULE_API CAVEStereoModule : public QObject, public IModule
    {
        Q_OBJECT

    public:
        MODULE_LOGGING_FUNCTIONS;
        CAVEStereoModule();
        ~CAVEStereoModule();
        virtual void PostInitialize();
        virtual void Update(f64 frametime);
        static const std::string &NameStatic() { return type_name_static_; }
        QVector<Ogre::RenderWindow*> GetCAVERenderWindows();

    public slots:
        void ShowStereoscopyWindow();
        void ShowCaveWindow();

    private:
        /// Type name of the module.
        static std::string type_name_static_;
        /// Manager for stereo
        StereoController* stereo_;
        /// Manager for CAVE
        CAVEManager* cave_;
    };
}

#endif
