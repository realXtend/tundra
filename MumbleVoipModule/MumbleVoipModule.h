// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_MumbleVoipModule_h
#define incl_MumbleVoipModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"
#include "MumbleVoipModuleApi.h"

#include <QObject>

namespace MumbleVoip
{
    class LinkPlugin;

    /**
	 *  Mumble support for Naali viewer.
	 *
     *  Offer user info with avatar and camera positions to Mumble client
	 *  using Mumble Link Plugin.
     *
	 *  In future the Mumble client will be integrated to this module.
     */
    class MUMBLE_VOIP_MODULE_API MumbleVoipModule : public QObject, public Foundation::ModuleInterfaceImpl
    {
        Q_OBJECT

    public:
        MumbleVoipModule();
        virtual ~MumbleVoipModule();

        void Load();
        void Unload();
        void Initialize();
        void PostInitialize();
        void Uninitialize();

        void Update(f64 frametime);
        bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

        //! Logging
        MODULE_LOGGING_FUNCTIONS
        static const std::string &NameStatic() { return module_name_; } //! returns name of this module. Needed for logging.

	protected:
		static std::string module_name_;

		//! Initialize Mumble Link plugin
		virtual void InitializeLinkPlugin();

		static const Foundation::Module::Type type_static_ = Foundation::Module::MT_Unknown;

        LinkPlugin* link_plugin_;
    };

} // end of namespace: MumbleVoip

#endif // incl_MumbleVoipModule_h
