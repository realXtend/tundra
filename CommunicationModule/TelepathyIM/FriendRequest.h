// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_TelepathyIM_FriendRequest_h
#define incl_Communication_TelepathyIM_FriendRequest_h

#include <TelepathyQt4/Contact>
#include <TelepathyQt4/PendingOperation>
//#include "interface.h"
#include "ModuleLoggingFunctions.h"
#include "FriendRequestInterface.h"
namespace TelepathyIM
{
    /**
     *  This class is only used by CommunicationService object. Do not use this
     *  directly!
     *
     *  A received friend request. This can be accepted or rejected. If Accpet() methos is called1
     *  then Connection object emits NewContact signal.
     *
     *
     */
    class FriendRequest : public Communication::FriendRequestInterface 
    {
        Q_OBJECT
        MODULE_LOGGING_FUNCTIONS
        static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

    public:
        FriendRequest(Tp::ContactPtr contact);
        virtual QString GetOriginatorName() const;
        virtual QString GetOriginatorID() const;
        virtual Communication::FriendRequestInterface::State GetState() const;
        virtual void Accept();
        virtual void Reject();
        virtual Tp::ContactPtr GetOriginatorTpContact();
    protected slots:
        virtual void OnPresencePublicationAuthorized(Tp::PendingOperation* op);
        virtual void OnPresenceSubscriptionResult(Tp::PendingOperation* op);
        virtual void OnPresenceSubscriptionChanged(Tp::Contact::PresenceState state);
    protected:

        State state_;
        Tp::ContactPtr tp_contact_;
    signals:
        //! When both the user and the target contact have published their presence 
        void Accepted(FriendRequest*request);
        void Canceled(FriendRequest*request);

    };
//    typedef std::vector<FriendRequest*> FriendRequestVector;

} // end of namespace: TelepathyIM

#endif // incl_Communication_TelepathyIM_FriendRequest_h