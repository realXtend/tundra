// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_FriendListItem_h
#define incl_Communication_FriendListItem_h

#include <QObject>
#include <QListWidget>
#include <QListWidgetItem>
#include <QIcon>
#include <QString>

//#include "interface.h"

namespace CommunicationUI
{
    class FriendListItem : public QObject, public QListWidgetItem
    {

    Q_OBJECT
    Q_PROPERTY(QString id_ WRITE SetID READ GetID)
    Q_PROPERTY(QString name_ WRITE SetName READ GetName)
    Q_PROPERTY(QString current_status_ WRITE SetStatus READ GetStatus)
    Q_PROPERTY(QString current_status_message_ WRITE SetStatusMessage READ GetStatusMessage)

    public:
        FriendListItem(QListWidget *parent_widget, QString id, QString name, QString status, QString status_message);
        virtual ~FriendListItem();

        //! Getters
        QString GetID()             { return id_; }
        QString GetName()           { return name_; }
        QString GetStatus()         { return current_status_; }
        QString GetStatusMessage()  { return current_status_message_; }

        //! Setters
        void SetID(QString id)                          { id_ = id; }
        void SetName(QString name)                      { name_ = name; }
        void SetStatus(QString status)                  { current_status_ = status; }
        void SetStatusMessage(QString status_message)   { current_status_message_ = status_message; }

    public slots:
        void UpdateFriendsStatus(const QString &new_status, const QString &new_message);

    private:
        QString id_;
        QString name_;
        QString current_status_;
        QString current_status_message_;
        
    };
}

#endif // incl_Communication_FriendListItem_h