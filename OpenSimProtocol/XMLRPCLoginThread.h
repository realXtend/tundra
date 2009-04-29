// For conditions of distribution and use, see copyright notice in license.txt

/// @file XMLRPCLoginThread.h
/// @brief

#ifndef incl_XMLRPCLoginThread_h
#define incl_XMLRPCLoginThread_h

#include "NetworkEvents.h"

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class XMLRPCLoginThread
    {
    public:
        XMLRPCLoginThread();
        ~XMLRPCLoginThread();
        
        /// Thread entry point.
        void operator()();
        
        /**
         * Sets up the XMP-RPC login procedure using authentication server or direct rexserver depending of 
         * given @p callMethod param and @p authentication param. All data which will be got from authentication of
		 * login into world server is saved in @p ConnectionThreadState struct. 
		 *
		 * @param first_name is username first part
		 * @param last_name is username last part (second name)
		 * @param password is a password which will be used to login into simulator and authentication server.
		 * @param worldAddress is a address of world (sim) server without port value. 
		 * @param worldPort is a port of world (sim) server. 
		 * @param callMethod is a function which will be "called" through xmlrpc-epi interface. Right now possible values are login_to_simulator 
		 * or ClientAuthentication.
		 * @param authentication_login is a login name (can be diffrent then a first_name + second_name). 
		 * @param authentication_address is a address to authentication server without port number. 
		 * @param authentication_port is a port of authentication server.
		 * @param authentication is a flag which defines is authentication done. 
         */
        void SetupXMLRPCLogin(
            const std::string& first_name, 
	        const std::string& last_name, 
	        const std::string& password,
	        const std::string& worldAddress,
	        const std::string& worldPort,
	        const std::string& callMethod,
            ConnectionThreadState *thread_state,
	        const std::string& authentication_login = "",
	        const std::string& authentication_address = "",
	        const std::string& authentication_port = "",
	        bool authentication = false);
        
        /// Performs the actual XML-RPC login procedure.
        ///@return true if login (or authentication) was successful.
        bool PerformXMLRPCLogin();

        /// Change the state of the XML-RPC worker.
        void SetConnectionState(Connection::State state) { threadState_->state = state; }
        
        ///@return State of connection.
        volatile Connection::State GetState() const;
        
        ///@return The client parameters retreived from the XML-RPC reply.
        const ClientParameters &GetClientParameters() const { return threadState_->parameters; }
        
        ///@return True, if the XML-RPC worker is ready.
        const bool IsReady() const { return ready_; }

    private:
        XMLRPCLoginThread(const XMLRPCLoginThread &);
        void operator=(const XMLRPCLoginThread &);

        /// Triggers the XML-RPC login procedure.
        bool beginLogin_;
        
        /// Indicates that the XML-RPC worker is ready to perform.
        bool ready_;
        
        /// Information which is received via the XML-RPC reply from the server.
        ConnectionThreadState *threadState_;
        
        /// Information needed for the XML-RPC login procedure.
        std::string firstName_;
        std::string lastName_;
        std::string password_;
        std::string worldAddress_;
        std::string worldPort_;
        std::string callMethod_;
        std::string authenticationLogin_;
        std::string authenticationAddress_;
        std::string authenticationPort_;
        bool authentication_;
    };
}

#endif
