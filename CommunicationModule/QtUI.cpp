#include "StableHeaders.h"
#include "Foundation.h"

#include "QtUI.h"

#include <QtUiTools>
#include <QFile>

namespace CommunicationUI
{

	/////////////////////////////////////////////////////////////////////
	// QtUI CLASS
	/////////////////////////////////////////////////////////////////////

	QtUI::QtUI(QWidget *parent, Foundation::Framework* framework)
		: QWidget(parent)
	{
		this->layout_ = new QVBoxLayout(this);
		loadUserInterface(false);
	}

	QtUI::~QtUI(void)
	{

	}

	void QtUI::loadUserInterface(bool connected)
	{
		// Remove widget if there is one
		if (widget_ != NULL && layout_->count() > 0)
			layout_->removeWidget(widget_);

		QUiLoader loader;
		if (connected)
		{
			// Init normal GUI
			QFile uiFile("./data/ui/communications.ui");
			this->widget_ = loader.load(&uiFile, this);
			this->widget_->setMinimumSize(472, 284);
			this->widget_->setMaximumSize(16000, 16000);
			uiFile.close();

			// Get widgets
			tabWidgetCoversations_ = this->widget_->findChild<QTabWidget *>("tabWidget_Conversations");
			tabWidgetCoversations_->clear();
			listWidgetFriends_ = this->widget_->findChild<QListWidget *>("listWidget_Friends");
			buttonSendMessage_ = this->widget_->findChild<QPushButton *>("pushButton_Send");
			buttonSendMessage_->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
			buttonSendMessage_->setIconSize(QSize(25,23));
			lineEditMessage_ = this->widget_->findChild<QLineEdit *>("lineEdit_Message");
			lineEditMessage_->setFocus(Qt::NoFocusReason);
			labelUsername_ = this->widget_->findChild<QLabel *>("label_UserName");
			lineEditStatus_ = this->widget_->findChild<QLineEdit *>("lineEdit_Status");
			comboBoxStatus_ = this->widget_->findChild<QComboBox *>("comboBox_Status");
			connectionStatus_ = this->widget_->findChild<QLabel *>("label_ConnectionStatus");
			connectionStatus_->setText("Connecting to server...");

			setAllEnabled(false);
			this->setWindowTitle("realXtend Communications");
		} 
		else
		{
			// Init login GUI
			Login *loginWidget_ = new Login(this, this->currentMessage);
			this->widget_ = loginWidget_->widget_;
			this->widget_->setMinimumSize(440, 157);
			this->widget_->setMaximumSize(440, 157);
			QObject::connect(loginWidget_, SIGNAL( userdataSet(QString, int, QString, QString) ), this, SLOT( connectToServer(QString, int, QString, QString) ));
			this->setWindowTitle("realXtend Naali Communications login");
		}

		this->layout_->setSpacing(0);
		this->layout_->setMargin(6);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
	}

	void QtUI::loadConnectedUserData(User *userData)
	{
		// USE WHEN THIS RETURNS ACTUAL DATA
		// labelUsername_->setText(QString(userData->GetUserID().c_str()));

		listWidgetFriends_->clear();
		ContactVector initialContacts = userData->GetContacts();
		ContactVector::const_iterator itr;
		for( itr=initialContacts.begin(); itr!=initialContacts.end(); itr++ )
		{
			Contact *contact = (*itr);
			ContactListItem *contactItem = new ContactListItem( QString(contact->GetRealName().c_str()),
																QString(contact->GetPresenceStatus().c_str()),
																QString(contact->GetPresenceMessage().c_str()), 
																contact );
			QObject::connect((QObject *)contact, SIGNAL( StateChanged() ), contactItem, SLOT( statusChanged() ));
			QObject::connect(listWidgetFriends_, SIGNAL( itemDoubleClicked(QListWidgetItem) ), this, SLOT( startNewChat(QListWidgetItem) )); 
			listWidgetFriends_->addItem(contactItem);
		}
	}

	void QtUI::setAllEnabled(bool enabled)
	{
		tabWidgetCoversations_->setEnabled(enabled);
		listWidgetFriends_->setEnabled(enabled);
		buttonSendMessage_->setEnabled(enabled);
		lineEditMessage_->setEnabled(enabled);
		labelUsername_->setEnabled(enabled);
		lineEditStatus_->setEnabled(enabled);
		comboBoxStatus_->setEnabled(enabled);
	}

	/////////////
	//  SLOTS  //
	/////////////

	void QtUI::connectToServer(QString server, int port, QString username, QString password)
	{
		loadUserInterface(true);
		labelUsername_->setText(username + "@" + server); // remove when username getting works
		connectionStatus_->setText("Initializing manager...");

		// Connect to IM server
		credentials.SetProtocol("jabber");
		credentials.SetUserID(username.toStdString());
		credentials.SetPassword(password.toStdString());
		credentials.SetServer(server.toStdString());
		credentials.SetServerPort(port);

		commManager_ = CommunicationManager::GetInstance();

		if (commManager_->GetState() == CommunicationManager::STATE_READY)
		{
			connectionStatus_->setText("Connecting...");
			managerReady();
		}	
		else if (commManager_->GetState() == CommunicationManager::STATE_ERROR)
		{
			LogWarning("[Communications UI] Comm manager in error state, IM login stopped");
			connectionStatus_->setText("Initializing manager...");
			QString message = "Communication manager initialize error."; 
			connectionFailed(message);
			return;
		}
		else
		{
			QObject::connect((QObject *)commManager_, SIGNAL( Ready() ), this, SLOT( managerReady() ));
			QObject::connect((QObject *)commManager_, SIGNAL( Error(QString &) ), this, SLOT( connectionFailed(QString &) ));
		}
	}

	void QtUI::managerReady()
	{
		LogDebug("[Communications UI] Comm manager ready");
		im_connection_ = commManager_->OpenConnection(credentials);

		QObject::connect((QObject*)im_connection_, SIGNAL( Connected() ), this, SLOT( connectionEstablished() ));
		QObject::connect((QObject*)im_connection_, SIGNAL( Error(QString &) ), this, SLOT( connectionFailed(QString &) ));
	}

	void QtUI::connectionEstablished()
	{
		connectionStatus_->setText("Connected");
		setAllEnabled(true);

		if ( im_connection_ != NULL && im_connection_->GetUser() != NULL)
		{
			this->loadConnectedUserData(im_connection_->GetUser());
		}
	}

	void QtUI::connectionFailed(QString &reason)
	{
		LogDebug("[Communications UI] Connection failed");
		this->currentMessage = reason;
		this->loadUserInterface(false);
	}

	void QtUI::startNewChat(QListWidgetItem *clickedItem)
	{
		LogDebug("[Communications UI] Starting new chat...");
		ContactListItem *listItem = (ContactListItem *)clickedItem;
		TextChatSessionPtr chatSession = im_connection_->CreateTextChatSession(listItem->contact_);
		
		Conversation *conversation = new Conversation(this->tabWidgetCoversations_, chatSession, listItem->contact_);

		tabWidgetCoversations_->addTab(conversation, QString(listItem->contact_->GetRealName().c_str()));
	}

	/////////////////////////////////////////////////////////////////////
	// LOGIN CLASS
	/////////////////////////////////////////////////////////////////////

	Login::Login(QWidget *parent, QString &message)
		: QWidget(parent)
	{
		initWidget(message);
		connectSignals();
		this->show();
	}

	Login::~Login()
	{

	}

	void Login::initWidget(QString &message)
	{
		// Init widget from .ui file
		QUiLoader loader;
		QFile uiFile("./data/ui/communications_login.ui");
		this->widget_ = loader.load(&uiFile, this);
		uiFile.close();

		// Get GUI elements
		labelStatus = this->widget_->findChild<QLabel *>("label_Status");
		if (!message.isEmpty())
			labelStatus->setText(message);
		textEditServer_ = this->widget_->findChild<QLineEdit *>("lineEdit_Server");
		textEditPort_ = this->widget_->findChild<QLineEdit *>("lineEdit_Port");
		textEditUsername_ = this->widget_->findChild<QLineEdit *>("lineEdit_Username");
		textEditPassword_ = this->widget_->findChild<QLineEdit *>("lineEdit_Password");
		buttonConnect_ = this->widget_->findChild<QPushButton *>("pushButton_Connect");
		buttonCancel_ = this->widget_->findChild<QPushButton *>("pushButton_Cancel");
	}

	void Login::connectSignals()
	{
		QObject::connect(this->buttonConnect_, SIGNAL( clicked(bool) ), this, SLOT ( checkInput(bool) ));
		QObject::connect(this->buttonCancel_, SIGNAL( clicked() ), this->parent(), SLOT( close() ));
	}

	void Login::checkInput(bool clicked)
	{
		QString serverUrl(this->textEditServer_->text());
		bool portSuccess = false;
		int port = this->textEditPort_->text().toInt(&portSuccess);
		QString username(this->textEditUsername_->text());
		QString password(this->textEditPassword_->text());

		if ( !serverUrl.isEmpty() && portSuccess && !username.isEmpty() && !password.isEmpty() )
		{
			emit( userdataSet(serverUrl, port, username, password) );
		}
	}


	/////////////////////////////////////////////////////////////////////
	// CONVERSATION CLASS
	/////////////////////////////////////////////////////////////////////

	Conversation::Conversation(QWidget *parent, TextChatSessionPtr chatSession, Contact *contact)
		: QWidget(parent), chatSession_(chatSession), contact_(contact)
	{
		initWidget();
		this->show();
	}

	Conversation::~Conversation()
	{

	}

	void Conversation::initWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/communications_conversation.ui");
		this->widget_ = loader.load(&uiFile, this);
		uiFile.close();

		textEditChat_ = this->widget_->findChild<QPlainTextEdit *>("textEdit_Chat");

		this->layout_ = new QVBoxLayout(this);
		this->layout_->setSpacing(0);
		this->layout_->setMargin(0);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
	}


	/////////////////////////////////////////////////////////////////////
	// CUSTOM QListWidgetItem CLASS
	/////////////////////////////////////////////////////////////////////

	ContactListItem::ContactListItem(QString &name, QString &status, QString &statusmessage, TpQt4Communication::Contact *contact)
		: QListWidgetItem(0, QListWidgetItem::UserType), name_(name), status_(status), statusmessage_(statusmessage), contact_(contact)
	{
		status_.append(" - " + statusmessage);
		this->setText(name_ + " (" + status_ + ")");
	}

	ContactListItem::~ContactListItem()
	{

	}

	void ContactListItem::updateItem()
	{
		this->setText(name_ + " (" + status_ + ")");
	}

	void ContactListItem::statusChanged()
	{
		status_ = QString(this->contact_->GetPresenceStatus().c_str());
		status_.append(" - ");
		status_.append(this->contact_->GetPresenceMessage().c_str());
		updateItem();
	}

} //end if namespace: CommunicationUI