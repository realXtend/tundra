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

QT_BEGIN_NAMESPACE
class QWebFrame;
QT_END_NAMESPACE

namespace RexLogic
{
    class RexLogicModule;

    class AbstractLoginHandler : public QObject
    {
        Q_OBJECT

    public:
        AbstractLoginHandler(Foundation::Framework *framework, RexLogicModule *rex_logic_module);
        virtual void InstantiateWorldSession() = 0;
        virtual void SetLoginNotifier(QObject *notifier) = 0;
        virtual QUrl ValidateServerUrl(QString urlString);

        ProtocolUtilities::LoginCredentialsInterface *credentials_;
        QUrl server_entry_point_url_;

        /// Pointer to Framework
        Foundation::Framework *framework_;

        /// Pointer to RexLogicModule
        RexLogicModule *rex_logic_module_;

    public slots:
        void Logout();
        void Quit();

    signals:
        void LoginStarted();
    };

    class OpenSimLoginHandler : public AbstractLoginHandler
    {
        Q_OBJECT

    public:
        OpenSimLoginHandler(Foundation::Framework *framework, RexLogicModule *rex_logic_module);
        virtual ~OpenSimLoginHandler();
        void InstantiateWorldSession();
        void SetLoginNotifier(QObject *notifier);

    public slots:
        void ProcessOpenSimLogin(QMap<QString, QString> map);
        void ProcessRealXtendLogin(QMap<QString, QString> map);

    private:
        //! Pointer to the opensim network interface.
        boost::weak_ptr<OpenSimProtocol::ProtocolModuleOpenSim> protocol_module_opensim_;
        OpenSimProtocol::OpenSimWorldSession *opensim_world_session_;
        OpenSimProtocol::RealXtendWorldSession *realxtend_world_session_;
    };

    class TaigaLoginHandler : public AbstractLoginHandler
    {
        Q_OBJECT

    public:
        TaigaLoginHandler(Foundation::Framework *framework, RexLogicModule *rex_logic_module);
        virtual ~TaigaLoginHandler();
        void InstantiateWorldSession();
        void SetLoginNotifier(QObject *notifier);

    public slots:
        void ProcessCommandParameterLogin(QString &entry_point_url);
        void ProcessWebLogin(QWebFrame *web_frame);
        void ProcessWebLogin(QString url);

    private:
        //! Pointer to the taiga network interface.
        boost::weak_ptr<TaigaProtocol::ProtocolModuleTaiga> protocol_module_taiga_;
        TaigaProtocol::TaigaWorldSession *taiga_world_session_;
    };
}

#endif //incl_RexLogic_LoginHandler_h
