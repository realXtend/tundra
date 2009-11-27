#ifndef incl_QtGUI_h
#define incl_QtGUI_h

#include <StableHeaders.h>
#include <UICanvas.h>
#include <QtGui>

#include "../interface.h"
#include "../Credentials.h"
#include "../CommunicationService.h"

using namespace QtUI;

namespace CommunicationUI
{
	// FRIEND REQUEST CLASS

	class FriendRequestUI : public QWidget
	{

	Q_OBJECT

	public:
		FriendRequestUI(QWidget *parent, Communication::FriendRequestInterface &request);
		FriendRequestUI(QWidget *parent, Communication::ConnectionInterface *connection);
		~FriendRequestUI(void);

	private slots:
		void ButtonHandlerAccept(bool clicked);
		void ButtonHandlerReject(bool clicked);
		void ButtonHandlerCloseWindow(bool clicked);
		void SendFriendRequest(bool clicked);

	private:
		Communication::FriendRequestInterface *request_;
		Communication::ConnectionInterface* connection_;
		
		QWidget *internalWidget_;
		QLabel *originator;
		QPushButton *accept;
		QPushButton *reject;
		QPushButton *askLater;
		QLineEdit *account;
		QLineEdit *message;

	signals:
		void CloseThisTab(FriendRequestUI *tabWidget);

	};

	// CUSTOM QTabWidget CLASS

	class ConversationsContainer : public QTabWidget
	{

	Q_OBJECT

	friend class UIContainer;
	friend class FriendRequestUI;

	public:
		ConversationsContainer(QWidget *parent);
		~ConversationsContainer(void);
		bool DoesTabExist(Communication::ContactInterface *contact);
        bool SetFocusTo(QString &tabName);
		
	public slots:
		void CloseFriendRequest(FriendRequestUI *request);

	private slots:
		void CloseTab(int index);

	};

	// CONVERSATION CLASS

	class Conversation : public QWidget
	{
	
	Q_OBJECT

	friend class UIContainer;
	friend class ConversationsContainer;

	public:
		Conversation(ConversationsContainer *parent, Communication::ChatSessionInterface &chatSession, Communication::ContactInterface *contact, QString name);
		~Conversation(void);

		void ShowMessageHistory(Communication::ChatMessageVector messageHistory);

	private:
		void InitWidget();
		void ConnectSignals();
		QString GenerateTimeStamp();
		void AppendLineToConversation(QString line);
		void AppendHTMLToConversation(QString html);

		QWidget *internalWidget_;
		QPlainTextEdit *textEditChat_;
		QLineEdit *lineEditMessage_;
		QString myName_;

		ConversationsContainer *myParent_;
		Communication::ChatSessionInterface& chat_session_;
		Communication::ContactInterface* contact_;

	private	slots:
		void OnMessageSent();
		void OnMessageReceived(const Communication::ChatMessageInterface &message);
		void ContactStateChanged(const QString &status, const QString &message);

	};

	// CUSTOM QListWidgetItem CLASS

	// UIContainer CLASS

	class UIContainer : public QWidget
	{
	
	Q_OBJECT

	friend class ConversationsContainer;

	MODULE_LOGGING_FUNCTIONS
	static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	public:
		UIContainer(QWidget *parent, Foundation::Framework *framework,
            boost::shared_ptr<UICanvas> canvas_login, boost::shared_ptr<UICanvas> canvas_chat);
		~UIContainer(void);

	public slots:
		void ConnectToServer(QString server, int port, QString username, QString password);
		void ManagerReady();
		void ConnectionEstablished(Communication::ConnectionInterface &connection);
		void ConnectionFailed(Communication::ConnectionInterface &connection);
		void ContactListChanged(Communication::ContactVector contacts);

		void StatusChanged(const QString &newStatus);
		void StatusMessageChanged();
		void StartNewChat(QListWidgetItem *clickedItem);
		void NewChatSessionRequest(Communication::ChatSessionInterface &);
		void NewFriendRequest(Communication::FriendRequestInterface &);
		void AddNewFriend(bool clicked);
		void RemoveFriend(bool clicked);
        void OnNewContact(const Communication::ContactInterface& contact);
        void OnContactRemoved(const Communication::ContactInterface& contact);

        // Notify system
        void NewMessageReceived(const Communication::ChatMessageInterface& message);
        void ShowUiAndNewMessage();
        void NotifyTimerUpdate();
        

	protected slots:
		void closeEvent(QCloseEvent *myCloseEvent);

	private:
		void LoadUserInterface(bool connected);
		void LoadConnectedUserData(Communication::ConnectionInterface *connection);
		QIcon GetStatusIcon(QString status);

		QString currentMessage;
		QWidget *loginWidget_;
		QWidget *chatWidget_;
		QListWidget *listWidgetFriends_;
		QLabel *labelUsername_;
		QLabel *labelLoginConnectionStatus_;
		QLabel *connectionStatus_;
		QLineEdit *lineEditStatus_;
		QComboBox *comboBoxStatus_;
		QPushButton *buttonAddFriend_;
		QPushButton *buttonRemoveFriend_;
		ConversationsContainer *tabWidgetCoversations_;

        // Notify system
        QWidget *popup_;
        QLabel *notify_message_;
        QLabel *notify_autohide_;
        QPushButton *notify_show_button_;
        QString tabname_;
        QTimer *notify_timer_;
        int timer_count_;

		Communication::Credentials credentials_;
		Communication::CommunicationServiceInterface* communication_service_;
		Communication::ConnectionInterface* im_connection_;
		Communication::ConnectionInterface* opensim_connection_;

        Foundation::Framework *framework_;
        boost::shared_ptr<UICanvas> canvas_login_;
        boost::shared_ptr<UICanvas> canvas_chat_;
        boost::shared_ptr<UICanvas> canvas_notify_popup_;

	signals:
        void ChangeToolBarButton(QString, QString);
		void Resized(QSize &);
		void DestroyCanvas();
		void SetCanvasTitle(QString title);
		void SetCanvasIcon(QIcon &icon);

	};

	// LOGIN CLASS

	class Login : public QWidget
	{
	
	Q_OBJECT

	friend class UIContainer;

	public:
		Login(QWidget *parent, QString &message, Foundation::Framework *framework);
		~Login(void);
        void SaveConfig();

	public slots:
		void CheckInput(bool clicked = false);

	private:
		void InitWidget(QString &message);
        void ReadConfig();
		void ConnectSignals();

		QWidget *internalWidget_;
		QLabel *labelStatus;
		QLineEdit *textEditServer_;
		QLineEdit *textEditPort_;
		QLineEdit *textEditUsername_;
		QLineEdit *textEditPassword_;
		QPushButton *buttonConnect_;
		QPushButton *buttonCancel_;

        Foundation::Framework *framework_;

	signals:
		void UserdataSet(QString, int, QString, QString);

	};


	class ContactListItem : public QObject, QListWidgetItem
	{

	Q_OBJECT

	friend class UIContainer;

	public:
		ContactListItem(QString &name, QString &status, QString &statusmessage, Communication::ContactInterface *contact);
		~ContactListItem(void);
        Communication::ContactInterface* GetContact() {return contact_;};

	public slots:
		void StatusChanged(const QString &status, const QString &message);

	private:
		void UpdateItem();

		QString name_;
		QString status_;
		QString statusmessage_;
		Communication::ContactInterface* contact_;
	};

	// QtGUI CLASS

	class QtGUI : public QObject
	{
	
	Q_OBJECT

	friend class UIContainer;
	
	MODULE_LOGGING_FUNCTIONS
	static const std::string NameStatic() { return "Communication IM UI"; } // for logging functionality

	public:
		QtGUI(Foundation::Framework *framework);
		~QtGUI(void);

	public slots:
		void ChangeToolBarButton(QString oldID, QString newID);
		void DestroyThis();

	private:
		Foundation::Framework *framework_;
		Communication::CommunicationServiceInterface* communication_service_;
		boost::shared_ptr<UICanvas> canvas_login_;
        boost::shared_ptr<UICanvas> canvas_chat_;
		UIContainer *UIContainer_;

	};


} //end if namespace: CommunicationUI

#endif // incl_QtGUI_h
