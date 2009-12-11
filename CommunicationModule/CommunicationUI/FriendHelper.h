// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_FriendHelper_h
#define incl_Communication_FriendHelper_h

#include "Foundation.h"
#include "ui_FriendListWidget.h"

#include <QObject>
#include <QMap>
#include <QString>

#include "interface.h"

namespace UiHelpers
{
    class FriendHelper : public QObject
    {

    Q_OBJECT

    public:
        FriendHelper();
        virtual ~FriendHelper();

        //! Setters
        void SetupUi(Ui::FrienListWidget *friend_list_ui) { SAFE_DELETE(friend_list_ui_); friend_list_ui_ = friend_list_ui; }

        //! Getters
        QMap<QString, Communication::ContactInterface*> GetContactsMap() { return contacts_map_; }

    public slots:
        void NewFriendRequest(Communication::FriendRequestInterface& friend_request);
        void OnNewContact(const Communication::ContactInterface& contact);
        void OnContactRemoved(const Communication::ContactInterface& contact);

        void UpdateFriendList(QMap<QString, Communication::ContactInterface*> contacts_map);

    private:
        Ui::FrienListWidget *friend_list_ui_;
        QMap<QString, Communication::ContactInterface*> contacts_map_;

    };
}

#endif // incl_Communication_FriendHelper_h