// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_OpensimIM_Contact_h
#define incl_Communication_OpensimIM_Contact_h

//#include "Foundation.h"
//#include "NetworkEvents.h"
//#include "interface.h"
#include "ContactInterface.h"

namespace OpensimIM
{
    /**
     *  @todo Initial state of online status is not checked currently.
     *        
     */ 
    class Contact : public Communication::ContactInterface
    {
    public:
        Contact(const QString &id, const QString &name);

        //! UUID of this contact
        virtual QString GetID() const;

        //! Provides name of this contact item
        virtual QString GetName() const;

        //! Not supported by Opensim
        virtual void SetName(const QString& name); 

        //! Values "online", "offline"
        virtual QString GetPresenceStatus() const;

        //! Not supported by Opensim
        virtual QString GetPresenceMessage() const;

        //! Connection object calls this according messages from server
        virtual void SetOnlineStatus(bool online);
    private:

        //! UUID of this contact
        QString id_;

        QString name_;

        bool online_;
        bool is_typing_; // probably will be used on ChatSessionParticipant class

        //bool rights_to_see_users_online_status;
        //bool rights_to_locate_user_;
        //bool right_to_use_users_objects_;
    };
    typedef std::vector<Contact*> ContactVector;

} // end of namespace: OpensimIM

#endif // incl_Communication_OpensimIM_Contact_h
