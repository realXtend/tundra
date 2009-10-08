#ifndef incl_QtGUI_h
#define incl_QtGUI_h

#include "StableHeaders.h"
#include "Foundation.h"
#include "UICanvas.h"

#include <QtGui>

#include "CommunicationManager.h"
#include "Connection.h"
#include "ChatSession.h"
#include "ChatSessionRequest.h"
#include "User.h"
#include "Contact.h"

using namespace TpQt4Communication;
using namespace QtUI;

namespace CommunicationUI
{
	// QtGUI CLASS

	class QtGUI : public QObject
	{
	
	Q_OBJECT

	friend class UIContainer;
	
	MODULE_LOGGING_FUNCTIONS
	static const std::string NameStatic() { return "CommunicationModule::QtGUI"; } // for logging functionality

	public:
		QtGUI(Foundation::Framework *framework);
		~QtGUI(void);

	public slots:
		void SetWindowSize(QSize &size);
		void DestroyThis();

	private:
		Foundation::Framework *framework_;
		CommunicationManager* commManager_;
		boost::shared_ptr<UICanvas> canvas_;
		UIContainer *UIContainer_;

	};

	// UIContainer CLASS

	class UIContainer : public QWidget
	{
	
	Q_OBJECT

	friend class ConversationsContainer;

	MODULE_LOGGING_FUNCTIONS
	static const std::string NameStatic() { return "CommunicationModule::UIController"; } // for logging functionality

	public:
		UIContainer(QWidget *parent);
		~UIContainer(void);

	public slots:
		void ConnectToServer(QString server, int port, QString username, QString password);
		void ManagerReady();
		void ConnectionEstablished();
		void ConnectionFailed(QString &reason);
		void ContactListChanged(ContactVector contacts);

		void StatusChanged(const QString &newStatus);
		void StatusMessageChanged();
		void StartNewChat(QListWidgetItem *clickedItem);
		void NewChatSessionRequest(ChatSessionRequest *);
		void NewFriendRequest(FriendRequest *request);
		void AddNewFriend(bool clicked);
		void RemoveFriend(bool clicked);

	protected slots:
		void closeEvent(QCloseEvent *myCloseEvent);

	private:
		void LoadUserInterface(bool connected);
		void LoadConnectedUserData(User *userData);
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

		Credentials credentials;
		CommunicationManager* commManager_;
		Connection* im_connection_;

	signals:
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
		Login(QWidget *parent, QString &message);
		~Login(void);

	public slots:
		void CheckInput(bool clicked);

	private:
		void InitWidget(QString &message);
		void ConnectSignals();

		QWidget *internalWidget_;
		QLabel *labelStatus;
		QLineEdit *textEditServer_;
		QLineEdit *textEditPort_;
		QLineEdit *textEditUsername_;
		QLineEdit *textEditPassword_;
		QPushButton *buttonConnect_;
		QPushButton *buttonCancel_;

	signals:
		void UserdataSet(QString, int, QString, QString);

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
		bool DoesTabExist(Contact *contact);
		
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
		Conversation(ConversationsContainer *parent, ChatSessionPtr chatSession, Contact *contact, QString name);
		~Conversation(void);

		void ShowMessageHistory(ChatMessageVector messageHistory);

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
		ChatSessionPtr chatSession_;
		Contact *contact_;

	private	slots:
		void OnMessageSent();
		void OnMessageReceived(ChatMessage &message);
		void ContactStateChanged();

	};

	// CUSTOM QListWidgetItem CLASS

	class ContactListItem : public QObject, QListWidgetItem
	{

	Q_OBJECT

	friend class UIContainer;

	public:
		ContactListItem(QString &name, QString &status, QString &statusmessage, Contact *contact);
		~ContactListItem(void);

	public slots:
		void StatusChanged();

	private:
		void UpdateItem();

		QString name_;
		QString status_;
		QString statusmessage_;
		Contact *contact_;

	};

	// FRIEND REQUEST CLASS

	class FriendRequestUI : public QWidget
	{

	Q_OBJECT

	public:
		FriendRequestUI::FriendRequestUI(QWidget *parent, FriendRequest *request);
		FriendRequestUI::FriendRequestUI(QWidget *parent, Connection *connection);
		FriendRequestUI::~FriendRequestUI(void);

	private slots:
		void ButtonHandlerAccept(bool clicked);
		void ButtonHandlerReject(bool clicked);
		void ButtonHandlerCloseWindow(bool clicked);
		void SendFriendRequest(bool clicked);

	private:
		FriendRequest *request_;
		Connection *connection_;
		
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

} //end if namespace: CommunicationUI

#endif // incl_QtGUI_h