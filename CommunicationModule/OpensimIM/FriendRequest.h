// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_OpensimIM_FriendRequest_h
#define incl_Communication_OpensimIM_FriendRequest_h

//#include <Foundation.h>
//#include "interface.h"
#include "FriendRequestInterface.h"

namespace Foundation
{
    class Framework;
}

namespace OpensimIM
{
    class FriendRequest : public Communication::FriendRequestInterface
    {
    public:
        FriendRequest(Foundation::Framework* framework, const QString &id, const QString &name, const QString &calling_card_folder_id);
        virtual QString GetOriginatorName() const;
        virtual QString GetOriginatorID() const;
        virtual State GetState() const;
        virtual void Accept();
        virtual void Reject();
    private:
        Foundation::Framework* framework_;
        State state_;
        QString friend_id_;
        QString name_;
        QString    calling_card_folder_id_;
    };
    typedef std::vector<FriendRequest*> FriendRequestVector;
    
} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_FriendRequest_h
