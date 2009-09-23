#ifndef incl_QtUI_h
#define incl_QtUI_h

#include "StableHeaders.h"
#include "Foundation.h"

#include <QtGui>
#include "CommunicationManager.h"
#include "Connection.h"

using namespace TpQt4Communication;

namespace CommunicationUI
{
	class QtUI : public QWidget
	{
	
	Q_OBJECT

	MODULE_LOGGING_FUNCTIONS
	static const std::string NameStatic() { return "CommunicationModule"; } // for logging functionality

	public:
		QtUI(QWidget *parent, Foundation::Framework* framework);
		~QtUI(void);

	public slots:
		void connectToServer(QString server, int port, QString username, QString password);
		void managerReady();
		void connectionEstablished();
		void connectionFailed(QString &reason);

	private:
		void loadUserInterface(bool connected);
		void loadConnectedUserData(User *userData);
		void setAllEnabled(bool enabled);

		QString currentMessage;
		QLayout *layout_;
		QWidget *widget_;
		QTabWidget *tabWidgetCoversations_;
		QListWidget *listWidgetFriends_;
		QPushButton *buttonSendMessage_;
		QLineEdit *lineEditMessage_;
		QLabel *labelUsername_;
		QLineEdit *lineEditStatus_;
		QComboBox *comboBoxStatus_;
		QLabel *connectionStatus_;

		Credentials credentials;
		CommunicationManager* commManager_;
		Connection* im_connection_;

	};

	// LOGIN CLASS

	class Login : public QWidget
	{
	
	Q_OBJECT

	friend class QtUI;

	public:
		Login(QWidget *parent, QString &message);
		~Login(void);

	public slots:
		void checkInput(bool clicked);

	private:
		void initWidget(QString &message);
		void connectSignals();

		QLabel *labelStatus;
		QLayout *layout_;
		QWidget *widget_;
		QLineEdit *textEditServer_;
		QLineEdit *textEditPort_;
		QLineEdit *textEditUsername_;
		QLineEdit *textEditPassword_;
		QPushButton *buttonConnect_;
		QPushButton *buttonCancel_;

	signals:
		void userdataSet(QString, int, QString, QString);

	};

	// CONVERSATION CLASS

	class Conversation : public QWidget
	{
	
	Q_OBJECT

	friend class QtUI;

	public:
		Conversation(QWidget *parent); // Add as inparam also the "conversation object" from mattiku
		~Conversation(void);

	private:
		void initWidget();

		QLayout *layout_;
		QWidget *widget_;
		QPlainTextEdit *textEditChat_;
		
	};

	// CUSTOM QListWidgetItem CLASS

	class ContactListItem : public QListWidgetItem
	{

	friend class QtUI;

	public:
		ContactListItem(QString &name, QString &status, QString &statusmessage, Contact *contact);
		~ContactListItem(void);

	private:
		QString name_;
		QString status_;
		QString statusmessage_;
		Contact *contact_;

	};

} //end if namespace: CommunicationUI

#endif // incl_QtUI_h