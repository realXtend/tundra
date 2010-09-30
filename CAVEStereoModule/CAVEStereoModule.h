// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CAVEStereoModule_CAVEStereoModule_h
#define incl_CAVEStereoModule_CAVEStereoModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "CAVEStereoModuleApi.h"
#include <QVector>

namespace Foundation
{
    class Framework;
}

//! Main class of the module.
namespace CAVEStereo
{
    class StereoController;
    class CAVEManager;
    class CAVESTEREO_MODULE_API CAVEStereoModule : public IModule
    {
    public:
        MODULE_LOGGING_FUNCTIONS;
        CAVEStereoModule();
        ~CAVEStereoModule();
        virtual void Load();
        virtual void PreInitialize();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);
        static const std::string &NameStatic() { return type_name_static_; }

        QVector<Ogre::RenderWindow*> GetCAVERenderWindows();
    private:
        //! Type name of the module.
        static std::string type_name_static_;
        //! Manager for stereo
        StereoController* stereo_;
        //! Manager for CAVE
        CAVEManager* cave_;
    };

}

#endif