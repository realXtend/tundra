// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "IModule.h"

#include <QObject>

namespace MediaPlayer
{
    class ServiceInterface;
    typedef boost::shared_ptr<ServiceInterface> ServicePtr; 
}

namespace PhononPlayer
{
    /**
     *  Offers PlayerService using Phonon backend.
     */
    class PhononPlayerModule : public QObject, public IModule
    {
        Q_OBJECT
    public:
        PhononPlayerModule();
        virtual ~PhononPlayerModule();

        // Module API
        virtual void Load();
        virtual void Unload();
        virtual void Initialize();
        virtual void PostInitialize();
        virtual void Uninitialize();
        virtual void Update(f64 frametime);

        static const std::string &NameStatic() { return type_name_static_; }

    private:
        void InitializeConsoleCommands();
        static std::string type_name_static_;
        
        MediaPlayer::ServicePtr player_service_;
    };

} // PhononPlayer

// incl_PhononPlayerModule_h
