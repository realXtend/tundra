//$ HEADER_NEW_FILE $ 
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  - Implementes and register TTSServiceInterface.
 *  - You must not use this module directly. All functionality must be used through service interface
 *
 *  @file   TtsModule.h
 *  @brief  TTS Module registers a TTS Service, integrating Festival Speech Synthesis
 *			System with realXtend.
 */

#ifndef incl_TtsModule_h
#define incl_TtsModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "Core.h"
#include "AttributeChangeType.h"
#include <QObject>

#include "TtsModuleApi.h"
#include "TtsServiceInterface.h"
#include "TtsService.h"

class RexUUID;

namespace Foundation
{
    class EventDataInterface;
}

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    typedef boost::weak_ptr<ProtocolModuleInterface> ProtocolWeakPtr;
}

namespace Scene
{
    class Entity;
}

class IComponent;

//class AttributeChange::Type;

namespace Tts
{
    class SettingsWidget;

    class TTS_MODULE_API TtsModule :  public QObject, public IModule
    {
        Q_OBJECT

    public:
        /// Default constructor.
        TtsModule();

        /// Destructor 
        virtual ~TtsModule();
        /// Load components: EC_TtsVoice
		/// It deletes tmp folder (if exists) and all the content and creates a new one \note This is need by Festival Speech Synthesis System
        void Load();
		/// Unload function: It removes tmp folder. 
		void UnLoad();
		/// Initializes and registers the TTS service
        void Initialize();
		/// PostInitialize function. Empty for now.
        void PostInitialize();
		/// Unregisters and destroys the service. 
        void Uninitialize();

        void SetupSettingsWidget();
		
        /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return module_name_; } 

		//! Logging
        MODULE_LOGGING_FUNCTIONS

	
    private:
        Q_DISABLE_COPY(TtsModule);

        /// Name of the module.
        static const std::string module_name_;
		/// TTS service
		TtsServicePtr tts_service_;
        SettingsWidget* settings_widget_;
    private slots:  
        void ConnectSceneSignals();
        void CheckNewComponent(Scene::Entity*, IComponent*, AttributeChange::Type);
    };
}  // end of namespace: Tts

#endif // incl_TtsModule_h
