#include "StableHeaders.h"
#include "Foundation.h"

#include "QtUI.h"
#include "UICanvas.h"
#include "QtModule.h"

#include <QtUiTools>
#include <QFile>
#include <QTime>

namespace CommunicationUI
{

	/////////////////////////////////////////////////////////////////////
	// QtUI CLASS
	/////////////////////////////////////////////////////////////////////

	QtUI::QtUI(QWidget *parent)
		: QWidget(parent)
	{
		this->setLayout(new QVBoxLayout);
		this->layout()->setMargin(0);
		this->setStyleSheet(QString("QLineEdit, QListView {color: rgb(130, 195, 255);background-color: rgb(255, 255, 255);selection-color: rgb(57, 87, 255);selection-background-color: rgb(255, 255, 255);border: 2px groove gray;border-color: rgb(19, 33, 95);border-radius: 15px;padding: 5px 10px;}"));
		loadUserInterface(false);
	}

	QtUI::~QtUI(void)
	{

	}

	void QtUI::loadUserInterface(bool connected)
	{
		if (connected)
		{
			if (loginWidget_ != NULL)
			{
				this->layout()->removeWidget(loginWidget_);
				delete loginWidget_;
			}

			// Init chat GUI
			QUiLoader loader;
			QFile uiFile("./data/ui/communications.ui");
			chatWidget_ = loader.load(&uiFile, this);
			chatWidget_->layout()->setMargin(9);
			uiFile.close();

			// Get widgets
			tabWidgetCoversations_ = findChild<QTabWidget *>("tabWidget_Conversations");
			tabWidgetCoversations_->clear();
			listWidgetFriends_ = findChild<QListWidget *>("listWidget_Friends");
			buttonSendMessage_ = findChild<QPushButton *>("pushButton_Send");
			buttonSendMessage_->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
			buttonSendMessage_->setIconSize(QSize(25,23));
			lineEditMessage_ = findChild<QLineEdit *>("lineEdit_Message");
			lineEditMessage_->setFocus(Qt::NoFocusReason);
			labelUsername_ = findChild<QLabel *>("label_UserName");
			lineEditStatus_ = findChild<QLineEdit *>("lineEdit_Status");
			lineEditStatus_->setText("status message not set..");
			comboBoxStatus_ = findChild<QComboBox *>("comboBox_Status");
			connectionStatus_ = findChild<QLabel *>("label_ConnectionStatus");
			connectionStatus_->setText("Connecting to server...");
			setAllEnabled(false);

			// Connect signals
			QObject::connect(lineEditMessage_, SIGNAL( returnPressed() ), this, SLOT( sendNewChatMessage() ));
			QObject::connect(buttonSendMessage_, SIGNAL( clicked() ), this, SLOT ( sendNewChatMessage() ));

			// Add widget to layout
			this->layout()->addWidget(chatWidget_);
			this->setWindowTitle("realXtend Communications");
			this->setMinimumSize(581, 262);
			this->setMaximumSize(16000, 16000);
		} 
		else
		{
			// Init login GUI
			loginWidget_ = new Login(this, currentMessage);
			// Connect signals
			QObject::connect(loginWidget_, SIGNAL( userdataSet(QString, int, QString, QString) ), this, SLOT( connectToServer(QString, int, QString, QString) ));
			// Add widget to layout
			this->layout()->addWidget(loginWidget_);
			this->setWindowTitle("realXtend Naali Communications login");
			this->setMinimumSize(450, 157);
			this->setMaximumSize(450, 157);
		}

	}

	void QtUI::loadConnectedUserData(User *userData)
	{
		// debug, use when you get this for real...
		//labelUsername_->setText(QString(userData->GetUserID().c_str())); 
		listWidgetFriends_->clear();
		ContactVector initialContacts = userData->GetContacts();
		ContactVector::const_iterator itrContacts;

		for( itrContacts=initialContacts.begin(); itrContacts!=initialContacts.end(); itrContacts++ )
		{
			Contact *contact = (*itrContacts);
			ContactListItem *contactItem = new ContactListItem( QString(contact->GetRealName().c_str()),
																QString(contact->GetPresenceStatus().c_str()),
																QString(contact->GetPresenceMessage().c_str()), 
																contact );
			QObject::connect((QObject *)contact, SIGNAL( StateChanged() ), contactItem, SLOT( statusChanged() ));
			listWidgetFriends_->addItem(contactItem);
		}

		// Get available state options
		PresenceStatusOptions options = im_connection_->GetAvailablePresenceStatusOptions();
		PresenceStatusOptions::const_iterator itrStatusOptions;
		comboBoxStatus_->clear();

		for( itrStatusOptions=options.begin(); itrStatusOptions!=options.end(); itrStatusOptions++ )
		{
			comboBoxStatus_->addItem( QString((*itrStatusOptions).c_str()) );
		}

		// Connect signals
		QObject::connect(listWidgetFriends_, SIGNAL( itemDoubleClicked(QListWidgetItem *) ), this, SLOT( startNewChat(QListWidgetItem *) )); 
		QObject::connect(comboBoxStatus_, SIGNAL( currentIndexChanged(const QString &) ), this, SLOT( statusChanged(const QString &) ));
		QObject::connect(lineEditStatus_, SIGNAL( returnPressed() ), this, SLOT( statusMessageChanged() ));
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

	// IM CONNECTION RELATED SLOTS //

	void QtUI::connectToServer(QString server, int port, QString username, QString password)
	{
		loadUserInterface(true);
		connectionStatus_->setText("Initializing manager...");
		labelUsername_->setText(username);

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
			connectionStatus_->setText("Initializing manager...");
			QString message = "Communication manager initialize error."; 
			connectionFailed(message);
		}
		else
		{
			QObject::connect((QObject *)commManager_, SIGNAL( Ready() ), this, SLOT( managerReady() ));
			QObject::connect((QObject *)commManager_, SIGNAL( Error(QString &) ), this, SLOT( connectionFailed(QString &) ));
		}

	}

	void QtUI::managerReady()
	{
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
			QObject::connect((QObject *)im_connection_, SIGNAL( ReceivedTextChatSessionRequest(TextChatSessionRequest *) ), this, SLOT( newChatSessionRequest(TextChatSessionRequest *) ));
		}
		else
		{
			connectionFailed(QString("Connection lost or userdata could not be retrieved"));
		}
	}

	void QtUI::connectionFailed(QString &reason)
	{
		LogDebug("[Communications UI] Connection failed");
		this->currentMessage = reason;
		this->loadUserInterface(false);
	}

	// GUI RELATED SLOTS //

	void QtUI::statusChanged(const QString &newStatus)
	{
		im_connection_->GetUser()->SetPresenceStatus( newStatus.toStdString() );
	}

	void QtUI::statusMessageChanged()
	{
		im_connection_->GetUser()->SetPresenceMessage( lineEditStatus_->text().toStdString() );
	}

	void QtUI::startNewChat(QListWidgetItem *clickedItem)
	{
		ContactListItem *listItem = (ContactListItem *)clickedItem;
		TextChatSessionPtr chatSession = im_connection_->CreateTextChatSession(listItem->contact_);
		Conversation *conversation = new Conversation(tabWidgetCoversations_, chatSession, listItem->contact_);
		tabWidgetCoversations_->addTab(conversation, QString(listItem->contact_->GetRealName().c_str()));
	}

	void QtUI::newChatSessionRequest(TextChatSessionRequest *chatSessionRequest)
	{
		LogInfo("newChatSessionRequest --------<-----<-----");
		TextChatSessionPtr chatSession = chatSessionRequest->Accept();
		if (chatSession.get() != NULL)
		{
			if (chatSessionRequest->GetOriginatorContact() != NULL )
			{
				Conversation *conversation = new Conversation(tabWidgetCoversations_, chatSession, chatSessionRequest->GetOriginatorContact());
				tabWidgetCoversations_->addTab(conversation, QString(chatSessionRequest->GetOriginatorContact()->GetRealName().c_str()));
			}
			else
			{
				LogWarning("newChatSessionRequest (handled successfully)");
			}
		}
		LogInfo("newChatSessionRequest (handled successfully)");
	}

	void QtUI::sendNewChatMessage()
	{
		QString message(lineEditMessage_->text());
		lineEditMessage_->clear();

		Conversation *conversation = (Conversation *)tabWidgetCoversations_->currentWidget();
		if (conversation != NULL)
		{
			conversation->chatSession_->SendTextMessage(message.toStdString());
			// TODO: wait for confirmation signal and then put text to own chat widget
			// now just put it there
			conversation->onMessageSent(message);
		}
	}

	/////////////////////////////////////////////////////////////////////
	// LOGIN CLASS
	/////////////////////////////////////////////////////////////////////

	Login::Login(QWidget *parent, QString &message)
		: QWidget(parent)
	{
		initWidget(parent, message);
		connectSignals();
	}

	Login::~Login()
	{

	}

	void Login::initWidget(QWidget *parent, QString &message)
	{
		// Init widget from .ui file
		QUiLoader loader;
		QFile uiFile("./data/ui/communications_login.ui");
		QWidget(loader.load(&uiFile, this));
		uiFile.close();

		// Get GUI elements
		labelStatus = findChild<QLabel *>("label_Status");
		if (!message.isEmpty())
			labelStatus->setText(message);
		textEditServer_ = findChild<QLineEdit *>("lineEdit_Server");
		textEditPort_ = findChild<QLineEdit *>("lineEdit_Port");
		textEditUsername_ = findChild<QLineEdit *>("lineEdit_Username");
		textEditPassword_ = findChild<QLineEdit *>("lineEdit_Password");
		buttonConnect_ = findChild<QPushButton *>("pushButton_Connect");
		buttonCancel_ = findChild<QPushButton *>("pushButton_Cancel");
	}

	void Login::connectSignals()
	{
		QObject::connect(buttonConnect_, SIGNAL( clicked(bool) ), this, SLOT ( checkInput(bool) ));
		QObject::connect(buttonCancel_, SIGNAL( clicked() ), this->parent(), SLOT( close() ));
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
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
		this->setStyleSheet(QString("QWidget {color: rgb(130, 195, 255);background-color: rgb(255, 255, 255);selection-color: rgb(57, 87, 255);selection-background-color: rgb(255, 255, 255);border: 2px groove gray;border-color: rgb(19, 33, 95);padding: 5px 5px;}"));
		initWidget();
		connectSignals();
	}

	Conversation::~Conversation()
	{

	}

	void Conversation::initWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/communications_conversation.ui");
		internalWidget = loader.load(&uiFile, this);
		this->layout()->addWidget(internalWidget);
		uiFile.close();
		
		textEditChat_ = findChild<QPlainTextEdit *>("textEdit_Chat");

		QString startMessage("Started chat session with ");
		startMessage.append(contact_->GetRealName().c_str());
		startMessage.append("...");
		appendLineToConversation(startMessage);
	}

	void Conversation::connectSignals()
	{
		QObject::connect((QObject*)chatSession_.get(), SIGNAL( MessageReceived(Message &) ), this, SLOT( onMessageReceived(Message &) ));
		QObject::connect((QObject*)contact_, SIGNAL( StateChanged() ), this, SLOT( contactStateChanged() ));
	}

	QString Conversation::generateTimeStamp()
	{
		return QTime::currentTime().toString();
	}


	void Conversation::appendLineToConversation(QString line)
	{
		textEditChat_->appendPlainText(line);
		
	}

	void Conversation::appendHTMLToConversation(QString html)
	{
		textEditChat_->appendHtml(html);
	}

	void Conversation::onMessageSent(QString message)
	{
		QString html("<span style='color:gray;'>[");
		html.append(generateTimeStamp());
		html.append("]</span> <span style='color:blue;'>me");
		/*html.append(((QtUI *)this->parent()->parent())->labelUsername_->text());*/
		html.append("</span><span style='color:black;'>: ");
		html.append(message);
		html.append("</span>");
		appendHTMLToConversation(html);
	}

	void Conversation::onMessageReceived(Message &message)
	{
		QString html("<span style='color:gray;'>[");
		html.append(generateTimeStamp());
		html.append("]</span> <span style='color:red;'>");
		html.append(contact_->GetRealName().c_str());
		html.append("</span><span style='color:black;'>: ");
		html.append(message.GetText().c_str());
		html.append("</span>");
		appendHTMLToConversation(html);
	}

	void Conversation::contactStateChanged()
	{
		QString html("<span style='color:gray;'>");
		html.append(contact_->GetRealName().c_str());
		html.append(" changed status to ");
		html.append(contact_->GetPresenceStatus().c_str());
		html.append("</span>");
		appendHTMLToConversation(html);
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