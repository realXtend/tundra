// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_LoginHandler_h
#define incl_RexLogic_LoginHandler_h

#include <boost/weak_ptr.hpp>

#include <QObject>
#include <QUrl>
#include <QMap>

namespace Foundation
{
    class Framework;
}

namespace TaigaProtocol
{
    class ProtocolModuleTaiga;
    class TaigaWorldSession;
}

namespace OpenSimProtocol
{
    class ProtocolModuleOpenSim;
    class OpenSimWorldSession;
    class RealXtendWorldSession;
}

namespace ProtocolUtilities
{
    class LoginCredentialsInterface;
}

class QWebFrame;

namespace RexLogic
{
    class RexLogicModule;

    class AbstractLoginHandler : public QObject
    {
        Q_OBJECT

    public:
        AbstractLoginHandler();
        virtual void InstantiateWorldSession() = 0;
        virtual void SetLoginNotifier(QObject *notifier) = 0;
        virtual QUrl ValidateServerUrl(QString &urlString);

        ProtocolUtilities::LoginCredentialsInterface *credentials_;
        QUrl server_entry_point_url_;

    public slots:
        virtual void Logout() = 0;
        virtual void Quit() = 0;

    signals:
        void LoginStarted();
    };

    class OpenSimLoginHandler : public AbstractLoginHandler
    {
        Q_OBJECT

    public:
        explicit OpenSimLoginHandler(RexLogicModule *owner);
        virtual ~OpenSimLoginHandler();
        void InstantiateWorldSession();
        void SetLoginNotifier(QObject *notifier);

    public slots:
        void ProcessOpenSimLogin(QMap<QString, QString> map);
        void ProcessRealXtendLogin(QMap<QString, QString> map);
        void Logout();
        void Quit();

    private:
        //! Pointer to the opensim network interface.
        boost::weak_ptr<OpenSimProtocol::ProtocolModuleOpenSim> protocol_module_opensim_;
        OpenSimProtocol::OpenSimWorldSession *opensim_world_session_;
        OpenSimProtocol::RealXtendWorldSession *realxtend_world_session_;
        /// Pointer to RexLogicModule
        RexLogicModule *owner_;
    };

    class TaigaLoginHandler : public AbstractLoginHandler
    {
        Q_OBJECT

    public:
        explicit TaigaLoginHandler(RexLogicModule *owner);
        virtual ~TaigaLoginHandler();
        void InstantiateWorldSession();
        void SetLoginNotifier(QObject *notifier);

    public slots:
        void ProcessCommandParameterLogin(QString &entry_point_url);
        void ProcessWebLogin(QWebFrame *web_frame);
        void ProcessWebLogin(QString url);
        void Logout();
        void Quit();

    private:
        //! Pointer to the taiga network interface.
        boost::weak_ptr<TaigaProtocol::ProtocolModuleTaiga> protocol_module_taiga_;
        TaigaProtocol::TaigaWorldSession *taiga_world_session_;
        /// Pointer to RexLogicModule
        RexLogicModule *owner_;
    };
}

#endif //incl_RexLogic_LoginHandler_h
