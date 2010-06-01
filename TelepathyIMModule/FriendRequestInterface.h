// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Comm_FriendRequestInterface_h
#define incl_Comm_FriendRequestInterface_h

#include <QObject>
#include <QString>

namespace Communication
{
    /**
     * A received friend request. This can be accepted or rejected. If Accept() methos is called,
     * then Connection object emits NewContact signal.
     */
    class FriendRequestInterface : public QObject
    {
        Q_OBJECT
    public:
        enum State { STATE_PENDING, STATE_ACCEPTED, STATE_REJECTED };
        virtual QString GetOriginatorName() const = 0;
        virtual QString GetOriginatorID() const = 0;
        virtual State GetState() const = 0;

    public slots:
        virtual void Accept() = 0;
        virtual void Reject() = 0;

    signals:
        //! When target have accepted the friend request
        void FriendRequestAccepted(const FriendRequestInterface* request); 

        //! If the protocol doesn't support this then no
        //! notification is send back about rejecting the friend request
        void FriendRequestRejected(const FriendRequestInterface* request); 
    };
}

#endif

