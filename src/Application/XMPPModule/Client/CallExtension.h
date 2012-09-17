/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   CallExtension.h
 *  @brief  Extension for XMPP:Client, provides p2p sip calls (XEP-0166, XEP-0167, XEP-0176)
 */

#ifndef incl_XMPP_CallExtension_h
#define incl_XMPP_CallExtension_h

#include "Extension.h"
#include "Call.h"

#include "qxmpp/QXmppCallManager.h"

#include <QObject>
#include <QString>

class Framework;

namespace XMPP
{
class Client;

//! Provides p2p voip calls. Implements XEP-0166, XEP-0167 & XEP-0176.
class CallExtension : public Extension
{
    Q_OBJECT
    Q_PROPERTY(QStringList calls READ GetCalls);
    Q_PROPERTY(QString activecall READ GetActiveCall);

public:
    CallExtension();
    virtual ~CallExtension();
    virtual void Initialize(Client *client);
    void Update(f64 frametime);

    enum CallTypeFlag { VoiceCall = 1, VideoCall = 2 };

public slots:
    /// Call remote user.
    /// \note Video calls not implemented due to lack of support in QXmpp
    /// \param callType bitfield containing flags defined in CallExtension::CallType enum
    /// \param peerResource Resource the call is connected to (must have voice-v1 capability)
    /// \param peerJid remote party's JabberID
    /// \return bool true on succesful call request
    bool CallUser(QString peerJid, QString peerResource, int callType);

    /// Script friendly overload
    /// \note Video calls not implemented due to lack of support in QXmpp
    /// \param peerJid remote party's JabberID
    /// \param peerResource Resource the call is connected to (must have voice-v1 capability)
    /// \param callType QStringList with 1-2 elements:
    ///         "Voice" for voice capability
    ///         "Video" for video capability
    ///         Empty list defaults to voice call
    /// \return bool true on succesful call request
    bool CallUser(QString peerJid, QString peerResource, QStringList callType = QStringList());

    //! Accept incoming call
    //! \param peerJid JabberID the call is associated with
    //! \return bool true on call found
    bool AcceptCall(QString peerJid);

    //! Disconnect call
    //! \param peerJid JabberID the call is associated with
    //! \return bool true on call found and disconnected
    bool DisconnectCall(QString peerJid);

    /// Sets active call on hold
    bool SuspendCall(QString peerJid);

    /// Returns list of suspended calls
    /// \note Doesn't contain currently active call
    /// \return QStringList containing JabberIDs associated with the calls
    QStringList GetCalls() const;

    //! Returns currently active call
    //! \return QString JabberID associated with the active call, empty for call not found
    QString GetActiveCall();

    //! Change active call
    //! \note Only one call can be active at a time,
    //!       previously active call will be suspended
    //! \return true on success
    bool SetActiveCall(QString peerJid);

private slots:
    void HandleCallReceived(QXmppCall* qxmppCall);
    void HandleCallStateChanged(Call::State state);
    void HandleCallDisconnected(Call *call);

private:
    static QString extension_name_;
    QXmppCallManager *qxmpp_call_manager_;
    Framework *framework_;
    Client *client_;
    QMap<QString, Call*> calls_;
    QMap<QString, Call*> incoming_calls_;

signals:
    void CallSuspended(QString peerJid);
    void CallDisconnected(QString peerJid);
    void CallIncoming(QString peerJid);
    void CallActive(QString peerJid);
};

} // end of namespace: XMPP

#endif // incl_XMPP_CallExtension_h
