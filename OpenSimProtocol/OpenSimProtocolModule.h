// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OpenSimProtocolModule_h
#define incl_OpenSimProtocolModule_h

#include "ModuleInterface.h"

#include "NetMessageManager.h"
#include "NetworkConnection.h"

#include <boost/shared_ptr.hpp>

using namespace Core;
using namespace boost;

namespace Foundation
{
   class Framework;
}

namespace OpenSimProtocol
{
    //! interface for modules
    class OpenSimProtocolModule : public Foundation::ModuleInterface_Impl
    {
    public:
        OpenSimProtocolModule();
        virtual ~OpenSimProtocolModule();

        virtual void Load();
        virtual void Unload();
        virtual void Initialize(Foundation::Framework *framework);
        virtual void Uninitialize(Foundation::Framework *framework);
        virtual void Update();
		
		/// Get state of the network module.
		const u8 GetNetworkState() const { return networkState_; }
		
		/// Get the message template filename from xml configuration file.
		//const std::string &GetTemplateFilename();
        
        ///
        //void AddListener(INetMessageListener *listener);
        
        //void Unregister(listener);
        
		/// State of the network module.
		enum NetworkState
		{	
			State_Connected,
			State_Disconnected,
		};

    private:
        Foundation::Framework *framework_;
        //! Poco xml configuration reader
        Poco::AutoPtr<Poco::Util::XMLConfiguration> config_;
		boost::shared_ptr<NetMessageManager> networkManager_;
		NetworkState networkState_;
    };
}

#endif
