//$ HEADER_NEW_FILE $ 
/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   NotificationTts.h
 *  @brief  Synthetizes notifications using TTS Service
 *			
 */

#ifndef incl_NotificationTts_h
#define incl_NotificationTts_h


#include "NotificationTtsApi.h"
//#include "ModuleInterface.h"
#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "Core.h"

#include <QObject>


class RexUUID;

namespace Foundation
{
    class EventDataInterface;
	class UiServiceInterface;
}

namespace ProtocolUtilities
{
    class ProtocolModuleInterface;
    typedef boost::weak_ptr<ProtocolModuleInterface> ProtocolWeakPtr;
}

namespace Tts
{
	class TtsServiceInterface;
}

namespace NotifiTts
{

    class NOTIFICATIONTTS_API NotificationTts :  public QObject, public IModule
    {
        Q_OBJECT

    public:
        /// Constructor.
        NotificationTts();
        /// Destructor 
        virtual ~NotificationTts();
        /// Load function
        void Load();
		/// Unload funcion
		void UnLoad();
		/// Connects notification signals with Notification2Speech slot
        void Initialize();
		/// Postinitialize
        void PostInitialize();
		/// Uninitialize
        void Uninitialize();
    
         /// Returns name of this module. Needed for logging.
        static const std::string &NameStatic() { return module_name_; } 

		//! Logging
        MODULE_LOGGING_FUNCTIONS

	
    private:
        Q_DISABLE_COPY(NotificationTts);

        /// Module name
        static const std::string module_name_;

		///TTS Service
		Tts::TtsServiceInterface* tts_service_;

        QString notification_voice_;
        bool enabled_;
	
	private slots:
		/** The message is synthetized using a default voice using TTS Service
		\param message Text to synthetize. */

		void Notification2Speech(const QString &message);

        void UpdateTtsSettings();
    };
}  // end of namespace

#endif // incl_NotificationTts_h
