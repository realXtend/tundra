// For conditions of distribution and use, see copyright notice in license.txt

/// @file OpenSimLoginThread.h
/// @brief @brief XML-RPC login worker.

#ifndef incl_OpenSimLoginThread_h
#define incl_OpenSimLoginThread_h

#include "NetworkEvents.h"
#include <QString>

namespace Foundation
{
    class Framework;
}

namespace OpenSimProtocol
{
    class OpenSimLoginThread
    {
    public:
        /// Default constructor.
        OpenSimLoginThread();

        /// Destructor.
        virtual ~OpenSimLoginThread();

        /// Set framework
        void SetFramework(Foundation::Framework* framework) { framework_  = framework; }

        /// Thread entry point.
        void operator()();

        /**
         * Sets up the XML-RPC login procedure using authentication server or direct rexserver depending of 
         * given @p callMethod param and @p authentication param. All data which will be got from authentication of
         * login into world server is saved in @p ConnectionThreadState struct. 
         *
         * @param first_name is username first part
         * @param last_name is username last part (second name)
         * @param password is a password which will be used to login into simulator and authentication server.
         * @param worldAddress is a address of world (sim) server without port value. 
         * @param worldPort is a port of world (sim) server. 
         */
        void PrepareOpenSimLogin(const QString &first_name,
                                 const QString &last_name,
                                 const QString &password,
                                 const QString &worldAddress,
                                 const QString &worldPort,
                                 ProtocolUtilities::ConnectionThreadState *thread_state);

        /**
         * Sets up the XML-RPC login procedure using authentication server or direct rexserver depending of 
         * given @p callMethod param and @p authentication param. All data which will be got from authentication of
         * login into world server is saved in @p ConnectionThreadState struct. 
         *
         * @param password is a password which will be used to login into simulator and authentication server.
         * @param worldAddress is a address of world (sim) server without port value. 
         * @param worldPort is a port of world (sim) server. 
         * @param authentication_login is a login name (can be diffrent then a first_name + second_name). 
         * @param authentication_address is a address to authentication server without port number. 
         * @param authentication_port is a port of authentication server.
         */
        void PrepareRealXtendLogin(
            const QString &password,
            const QString &worldAddress,
            const QString &worldPort,
            ProtocolUtilities::ConnectionThreadState *thread_state,
            const QString &authentication_login,
            const QString &authentication_address,
            const QString &authentication_port);

        /// Performs the actual XML-RPC login procedure.
        ///@return true if login (or authentication) was successful.
        bool PerformXMLRPCLogin();

        /// Change the state of the XML-RPC worker.
        void SetConnectionState(ProtocolUtilities::Connection::State state) { threadState_->state = state; }

        ///@return State of connection.
        volatile ProtocolUtilities::Connection::State GetState() const;

        ///@return error message as std::string
        std::string &GetErrorMessage() const;

        ///@return The client parameters retreived from the XML-RPC reply.
        const ProtocolUtilities::ClientParameters &GetClientParameters() const { return threadState_->parameters; }

        ///@return True, if the XML-RPC worker is ready.
        const bool IsReady() const { return ready_; }

    private:
        OpenSimLoginThread(const OpenSimLoginThread &);
        void operator=(const OpenSimLoginThread &);

        /// Triggers the XML-RPC login procedure.
        bool start_login_;

        /// Indicates that the XML-RPC worker is ready to perform.
        bool ready_;

        /// Information which is received via the XML-RPC reply from the server.
        ProtocolUtilities::ConnectionThreadState *threadState_;

        /// Framework pointer
        Foundation::Framework* framework_;

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
        std::string authentication_;

        static std::string LOGIN_TO_SIMULATOR;
        static std::string CLIENT_AUTHENTICATION;
        static std::string OPTIONS;
        static std::string REALXTEND_AUTHENTICATION;
        static std::string OPENSIM_AUTHENTICATION;
    };
}

#endif
