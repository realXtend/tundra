#include "StableHeaders.h"
#include "Foundation.h"
#include "QtModule.h"

#include <QtUiTools>
#include <QFile>
#include <QTime>

#include "QtGUI.h"

namespace CommunicationUI
{

	QtGUI::QtGUI(Foundation::Framework *framework)
		: framework_(framework)
	{
		LogInfo("Loading UIController to QtModule UICanvas...");
		// Get comm manager to check state
		commManager_ = CommunicationManager::GetInstance();
		// Use QtModule to show our QtUI widget
		boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
		
		if ( qt_module.get() == 0)
		{	
			LogWarning("Could not aqquire QtModule and show Comm UI");
		}
		else if ( commManager_->GetState() != CommunicationManager::STATE_ERROR )
		{
			// Set param to QtUI::UICanvas::Internal to put inside ogre window
			canvas_ = qt_module->CreateCanvas(UICanvas::External).lock();
			UIContainer *UIContainer_ = new UIContainer(0);
			canvas_->AddWidget(UIContainer_);
			canvas_->Show(); 

			// Connect signal for resizing
			QObject::connect(UIContainer_, SIGNAL( Resized(QSize &) ), this, SLOT( SetWindowSize(QSize &) ));
			QObject::connect(UIContainer_, SIGNAL( DestroyCanvas() ), this, SLOT( DestroyThis() ));
			QObject::connect(UIContainer_, SIGNAL( SetCanvasTitle(QString &) ), canvas_.get(), SLOT( SetCanvasWindowTitle(QString &) ));
			QObject::connect(UIContainer_, SIGNAL( SetCanvasIcon(QIcon &) ), canvas_.get(), SLOT( SetCanvasWindowIcon(QIcon &) ));
			
			// Init title, icon and size
			canvas_->SetCanvasWindowIcon(QIcon(":/images/iconUsers.png"));
			canvas_->SetCanvasWindowTitle(QString("realXtend Naali Communications Login"));
			SetWindowSize(QSize(450, 165));

			LogInfo("Loading succesfull");
		}		
		else
		{
			LogWarning("Communication manager is in a error state, skip showing GUI");
		}
	}

	QtGUI::~QtGUI(void)
	{
		DestroyThis();
	}

	void QtGUI::DestroyThis()
	{
		boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
		QtUI::QtModule *qt_ui = dynamic_cast<QtModule*>(qt_module.get());
		if (qt_ui != 0)
		{
			canvas_->Hide();
			qt_ui->DeleteCanvas(canvas_);
		}
	}

	void QtGUI::SetWindowSize(QSize &newSize)
	{
		canvas_->SetCanvasSize(newSize.width(), newSize.height());
	}


	/////////////////////////////////////////////////////////////////////
	// UIContainer CLASS
	/////////////////////////////////////////////////////////////////////

	UIContainer::UIContainer(QWidget *parent)
		: QWidget(parent), chatWidget_(0), loginWidget_(0)
	{
		commManager_ = CommunicationManager::GetInstance();
		QObject::connect((QObject *)commManager_, SIGNAL( Error(QString &) ), this, SLOT( ConnectionFailed(QString &) ));

		LogInfo("Creating UIContainer, initializing with Login widget...");
		this->setLayout(new QVBoxLayout);
		this->layout()->setMargin(0);
		LoadUserInterface(false);
		LogInfo("Loading successfull");
	}

	UIContainer::~UIContainer(void)
	{
		delete this->layout();
	}

	void UIContainer::LoadUserInterface(bool connected)
	{
		if (connected)
		{
			if (loginWidget_ != NULL)
			{
				this->layout()->removeWidget(loginWidget_);
				delete loginWidget_;
				loginWidget_ = 0;
			}
			else if (chatWidget_ != NULL)
			{
				this->layout()->removeWidget(chatWidget_);
				delete chatWidget_;
				chatWidget_ = 0;
			}

			// Init chat GUI
			QUiLoader loader;
			QFile uiFile("./data/ui/communications.ui");
			chatWidget_ = loader.load(&uiFile, this);
			chatWidget_->layout()->setMargin(9);
			uiFile.close();

			// Insert custom tab widget
			tabWidgetCoversations_ = new ConversationsContainer(this);
			tabWidgetCoversations_->clear();
			QHBoxLayout *friendsAndTabWidget = findChild<QHBoxLayout *>("horizontalLayout_FriendsAndTabWidget");
			friendsAndTabWidget->insertWidget(1, tabWidgetCoversations_); 
			friendsAndTabWidget->setStretch(1,2);
			
			// Get widgets
			listWidgetFriends_ = findChild<QListWidget *>("listWidget_Friends");
			listWidgetFriends_->setIconSize(QSize(10,10));
			labelUsername_ = findChild<QLabel *>("label_UserName");
			lineEditStatus_ = findChild<QLineEdit *>("lineEdit_Status");
			comboBoxStatus_ = findChild<QComboBox *>("comboBox_Status");
			comboBoxStatus_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			comboBoxStatus_->setIconSize(QSize(10,10));
			buttonAddFriend_ = findChild<QPushButton *>("pushButton_AddFriend");
			buttonAddFriend_->setIcon(QIcon(":/images/iconAdd.png"));
			buttonAddFriend_->setLayoutDirection(Qt::RightToLeft);
			buttonAddFriend_->setIconSize(QSize(10,10));
			buttonRemoveFriend_ = findChild<QPushButton *>("pushButton_RemoveFriend");
			buttonRemoveFriend_->setIcon(QIcon(":/images/iconRemove.png"));
			buttonRemoveFriend_->setIconSize(QSize(10,10));

			// Add widget to layout
			this->layout()->addWidget(chatWidget_);
			emit( SetCanvasTitle(QString("realXtend Naali Communications")) );
			this->setMinimumSize(600, 262);
			emit ( Resized(QSize(600, 262)) );
		} 
		else
		{
			if (chatWidget_ != NULL)
			{
				this->layout()->removeWidget(chatWidget_);
				delete chatWidget_;
				chatWidget_ = 0;
			} 
			else if (loginWidget_ != NULL)
			{
				this->layout()->removeWidget(loginWidget_);
				delete loginWidget_;
				loginWidget_ = 0;
			}

			// Init login GUI
			loginWidget_ = new Login(this, currentMessage);
			// Get widgets
			labelLoginConnectionStatus_ = findChild<QLabel *>("label_Status");
			// Connect signals
			QObject::connect(loginWidget_, SIGNAL( UserdataSet(QString, int, QString, QString) ), this, SLOT( ConnectToServer(QString, int, QString, QString) ));
			// Add widget to layout
			this->layout()->addWidget(loginWidget_);
			emit( SetCanvasTitle(QString("realXtend Naali Communications Login")) );
			this->setMinimumSize(450, 165);
			emit ( Resized(QSize(450, 165)) );
		}

	}

	void UIContainer::LoadConnectedUserData(User *userData)
	{
		labelUsername_->setText(QString(userData->GetUserID().c_str())); 
		
		// Update contacts
		ContactListChanged(userData->GetContacts());

		// Get available state options
		PresenceStatusOptions options = im_connection_->GetAvailablePresenceStatusOptions();
		PresenceStatusOptions::const_iterator itrStatusOptions;
		QString status;
		comboBoxStatus_->clear();

		for( itrStatusOptions=options.begin(); itrStatusOptions!=options.end(); itrStatusOptions++ )
		{
			status = QString((*itrStatusOptions).c_str());
			comboBoxStatus_->addItem(GetStatusIcon(status), status);
		}

		// Connect signals
		//QObject::connect(im_connection_->GetUser(), SIGNAL ( NAME_THIS(ContactVector) ), this, SLOT( ContactListChanged(ContactVector) ));
		QObject::connect(listWidgetFriends_, SIGNAL( itemDoubleClicked(QListWidgetItem *) ), this, SLOT( StartNewChat(QListWidgetItem *) )); 
		QObject::connect(comboBoxStatus_, SIGNAL( currentIndexChanged(const QString &) ), this, SLOT( StatusChanged(const QString &) ));
		QObject::connect(lineEditStatus_, SIGNAL( returnPressed() ), this, SLOT( StatusMessageChanged() ));
		QObject::connect(buttonAddFriend_, SIGNAL( clicked(bool) ), this, SLOT( AddNewFriend(bool) ));
		QObject::connect(buttonRemoveFriend_, SIGNAL( clicked(bool) ), this, SLOT( RemoveFriend(bool) ));

	}

	QIcon UIContainer::GetStatusIcon(QString status)
	{
		if ( QString::compare(status, QString("available"), Qt::CaseInsensitive) == 0 )
			return QIcon(":/images/iconGreen.png");
		else if ( QString::compare(status, QString("offline"), Qt::CaseInsensitive) == 0 )
			return QIcon(":/images/iconRed.png");
		else if ( QString::compare(status, QString("away"), Qt::CaseInsensitive) == 0 ||
				  QString::compare(status, QString("hidden"), Qt::CaseInsensitive) == 0 )
			return QIcon(":/images/iconOrange.png");
		else if ( QString::compare(status, QString("chat"), Qt::CaseInsensitive) == 0 || 
				  QString::compare(status, QString("dnd"), Qt::CaseInsensitive) == 0 || 
				  QString::compare(status, QString("xa"), Qt::CaseInsensitive) == 0 )
			return QIcon(":/images/iconBlue.png");
		else
			return QIcon();
	}

	/////////////
	//  SLOTS  //
	/////////////

	// IM CONNECTION RELATED SLOTS //

	void UIContainer::ConnectToServer(QString server, int port, QString username, QString password)
	{
		labelLoginConnectionStatus_->setText("Initializing manager...");

		// Connect to IM server
		credentials.SetProtocol("jabber");
		credentials.SetUserID(username.toStdString());
		credentials.SetPassword(password.toStdString());
		credentials.SetServer(server.toStdString());
		credentials.SetServerPort(port);

		if (commManager_->GetState() == CommunicationManager::STATE_READY)
		{
			labelLoginConnectionStatus_->setText("Connecting...");
			ManagerReady();
		}	
		else if (commManager_->GetState() == CommunicationManager::STATE_ERROR)
		{
			labelLoginConnectionStatus_->setText("Initializing manager...");
			QString message = "Communication manager initialize error."; 
			ConnectionFailed(message);
		}
		else
		{
			QObject::connect((QObject *)commManager_, SIGNAL( Ready() ), this, SLOT( ManagerReady() ));
			QObject::connect((QObject *)commManager_, SIGNAL( Error(QString &) ), this, SLOT( ConnectionFailed(QString &) ));
		}

	}

	void UIContainer::ManagerReady()
	{
		im_connection_ = commManager_->OpenConnection(credentials);

		QObject::connect((QObject*)im_connection_, SIGNAL( Connected() ), this, SLOT( ConnectionEstablished() ));
		QObject::connect((QObject*)im_connection_, SIGNAL( Error(QString &) ), this, SLOT( ConnectionFailed(QString &) ));
	}

	void UIContainer::ConnectionEstablished()
	{
		labelLoginConnectionStatus_->setText("Connected");
		
		if ( im_connection_ != NULL && im_connection_->GetUser() != NULL)
		{
			QObject::connect((QObject *)im_connection_, SIGNAL( ReceivedChatSessionRequest(ChatSessionRequest *) ), this, SLOT( NewChatSessionRequest(ChatSessionRequest *) ));
			QObject::connect((QObject *)im_connection_, SIGNAL( ReceivedFriendRequest(FriendRequest *) ), this, SLOT( NewFriendRequest(FriendRequest *) ));
			LoadUserInterface(true);
			LoadConnectedUserData(im_connection_->GetUser());
		}
		else
		{
			ConnectionFailed(QString("Connection lost or userdata could not be retrieved"));
		}
	}

	void UIContainer::ConnectionFailed(QString &reason)
	{
		LogDebug("[Communications UI] Connection failed");
		this->currentMessage = reason;
		this->LoadUserInterface(false);
	}

	void UIContainer::ContactListChanged(ContactVector contacts)
	{
		listWidgetFriends_->clear();
		ContactVector::const_iterator itrContacts;

		for( itrContacts=contacts.begin(); itrContacts!=contacts.end(); itrContacts++ )
		{
			Contact *contact = (*itrContacts);
			ContactListItem *contactItem = new ContactListItem( QString(contact->GetName().c_str()),
																QString(contact->GetPresenceStatus().c_str()),
																QString(contact->GetPresenceMessage().c_str()), 
																contact );
			QObject::connect((QObject *)contact, SIGNAL( StateChanged() ), contactItem, SLOT( StatusChanged() ));
			listWidgetFriends_->addItem(contactItem);
		}
	}

	// GUI RELATED SLOTS //

	void UIContainer::StatusChanged(const QString &newStatus)
	{
		im_connection_->GetUser()->SetPresenceStatus( newStatus.toStdString() );
	}

	void UIContainer::StatusMessageChanged()
	{
		im_connection_->GetUser()->SetPresenceMessage( lineEditStatus_->text().toStdString() );
	}

	void UIContainer::StartNewChat(QListWidgetItem *clickedItem)
	{
		ContactListItem *listItem = (ContactListItem *)clickedItem;
		if ( tabWidgetCoversations_->DoesTabExist(listItem->contact_) == false )
		{
			ChatSessionPtr chatSession = im_connection_->CreateChatSession(listItem->contact_);
			Conversation *conversation = new Conversation(tabWidgetCoversations_, chatSession, listItem->contact_, QString(im_connection_->GetUser()->GetName().c_str()));
			tabWidgetCoversations_->addTab(conversation, clickedItem->icon(), QString(listItem->contact_->GetName().c_str()));
			tabWidgetCoversations_->setCurrentWidget(conversation);
		}
	}

	void UIContainer::NewChatSessionRequest(ChatSessionRequest *chatSessionRequest)
	{
		ChatSessionPtr chatSession = chatSessionRequest->Accept();
		
		if (chatSession.get() != NULL)
		{
			if (chatSessionRequest->GetOriginatorContact() != NULL )
			{
				if ( tabWidgetCoversations_->DoesTabExist(chatSessionRequest->GetOriginatorContact()) == false )
				{
					Conversation *conversation = new Conversation(tabWidgetCoversations_, chatSession, chatSessionRequest->GetOriginatorContact(), QString(im_connection_->GetUser()->GetName().c_str()));
					conversation->ShowMessageHistory(chatSession->GetMessageHistory());
					tabWidgetCoversations_->addTab(conversation, GetStatusIcon(QString(chatSessionRequest->GetOriginatorContact()->GetPresenceStatus().c_str())), QString(chatSessionRequest->GetOriginatorContact()->GetName().c_str()));		
					tabWidgetCoversations_->setCurrentWidget(conversation);
				}
			}
			else
			{
				LogWarning("NewChatSessionRequest (handled successfully)");
			}
		}
		LogInfo("NewChatSessionRequest (handled successfully)");
	}

	void UIContainer::NewFriendRequest(FriendRequest *request)
	{
		LogInfo("NewFriendRequest recieved");
		FriendRequestUI *friendRequest = new FriendRequestUI(this, request);
		tabWidgetCoversations_->addTab(friendRequest, QString("New Friend Request"));
		tabWidgetCoversations_->setCurrentWidget(friendRequest);
		QObject::connect(friendRequest, SIGNAL( CloseThisTab(FriendRequestUI *) ), tabWidgetCoversations_, SLOT( CloseFriendRequest(FriendRequestUI *) ));
	}

	void UIContainer::AddNewFriend(bool clicked)
	{
		LogInfo("AddNewFriend clicked");
		FriendRequestUI *friendRequest = new FriendRequestUI(this, im_connection_);
		tabWidgetCoversations_->addTab(friendRequest, QString("Add New Friend"));
		tabWidgetCoversations_->setCurrentWidget(friendRequest);
		QObject::connect(friendRequest, SIGNAL( CloseThisTab(FriendRequestUI *) ), tabWidgetCoversations_, SLOT( CloseFriendRequest(FriendRequestUI *) ));
	}

	void UIContainer::RemoveFriend(bool clicked)
	{
		LogInfo("RemoveFriend clicked");
		int row = listWidgetFriends_->currentRow();
		ContactListItem *taken = dynamic_cast<ContactListItem *>(listWidgetFriends_->takeItem(listWidgetFriends_->currentRow()));
		delete taken;
		taken = 0;
	}

	void UIContainer::closeEvent(QCloseEvent *myCloseEvent) 
	{
		LogInfo("CloseEvent catched");
		emit ( DestroyCanvas() );
		QWidget::closeEvent(myCloseEvent);
	}


	/////////////////////////////////////////////////////////////////////
	// LOGIN CLASS
	/////////////////////////////////////////////////////////////////////

	Login::Login(QWidget *parent, QString &message)
		: QWidget(parent)
	{
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
		InitWidget(message);
		ConnectSignals();
	}

	Login::~Login()
	{

	}

	void Login::InitWidget(QString &message)
	{
		// Init widget from .ui file
		QUiLoader loader;
		QFile uiFile("./data/ui/communications_login.ui");
		internalWidget_ = loader.load(&uiFile, this);
		internalWidget_->layout()->setMargin(9);
		this->layout()->addWidget(internalWidget_);
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

	void Login::ConnectSignals()
	{
		QObject::connect(buttonConnect_, SIGNAL( clicked(bool) ), this, SLOT ( CheckInput(bool) ));
		QObject::connect(buttonCancel_, SIGNAL( clicked() ), this->parent(), SLOT( close() ));
	}

	void Login::CheckInput(bool clicked)
	{
		QString serverUrl(this->textEditServer_->text());
		bool portSuccess = false;
		int port = this->textEditPort_->text().toInt(&portSuccess);
		QString username(this->textEditUsername_->text());
		QString password(this->textEditPassword_->text());

		if ( !serverUrl.isEmpty() && portSuccess && !username.isEmpty() && !password.isEmpty() )
		{
			emit( UserdataSet(serverUrl, port, username, password) );
		}
	}


	/////////////////////////////////////////////////////////////////////
	// CUSTOM QTabWidget CLASS
	/////////////////////////////////////////////////////////////////////

	ConversationsContainer::ConversationsContainer(QWidget *parent)
		: QTabWidget(parent)
	{
		// Init widget to wanted state (not drawing QTabWidget or QTabBar background)
		setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		setStyleSheet(QString("background-color: rgba(255, 255, 255, 0);"));
		setDocumentMode(true);
		setMovable(true);
		setTabsClosable(true);
		tabBar()->setDocumentMode(true);
		tabBar()->setDrawBase(false);
		tabBar()->setIconSize(QSize(10,10));
		// Connect signal
		QObject::connect(this, SIGNAL( tabCloseRequested(int) ), this, SLOT( CloseTab(int) ));
	}

	ConversationsContainer::~ConversationsContainer()
	{

	}

	void ConversationsContainer::CloseFriendRequest(FriendRequestUI *request)
	{
		this->removeTab(this->indexOf(request));
		delete request;
	}

	void ConversationsContainer::CloseTab(int index)
	{
		QWidget *child = this->widget(index);
		Conversation *childConversation = dynamic_cast<Conversation *>(child);
		if (childConversation != 0)
			childConversation->chatSession_->Close();
		this->removeTab(index);
		delete child;
		child = 0;
	}

	bool ConversationsContainer::DoesTabExist(Contact *contact)
	{
		for (int i=0; i<this->count(); i++)
		{
			if ( QString::compare(this->tabText(i), QString(contact->GetName().c_str())) == 0 )
			{
				this->setCurrentIndex(i);
				return true;
			}
		}
		return false;
	}


	/////////////////////////////////////////////////////////////////////
	// CONVERSATION CLASS
	/////////////////////////////////////////////////////////////////////

	Conversation::Conversation(ConversationsContainer *parent, ChatSessionPtr chatSession, Contact *contact, QString name)
		: QWidget(parent), myParent_(parent), chatSession_(chatSession), contact_(contact), myName_(name)
	{
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
		InitWidget();
		ConnectSignals();
	}

	Conversation::~Conversation()
	{

	}

	void Conversation::InitWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/communications_conversation.ui");
		internalWidget_ = loader.load(&uiFile, this);
		this->layout()->addWidget(internalWidget_);
		uiFile.close();
		
		textEditChat_ = findChild<QPlainTextEdit *>("textEdit_Chat");
		lineEditMessage_ = findChild<QLineEdit *>("lineEdit_Message");
		lineEditMessage_->setFocus(Qt::NoFocusReason);

		QString startMessage("Started chat session with ");
		startMessage.append(contact_->GetName().c_str());
		startMessage.append("...");
		AppendLineToConversation(startMessage);
	}

	void Conversation::ConnectSignals()
	{
		QObject::connect((QObject*)chatSession_.get(), SIGNAL( MessageReceived(ChatMessage &) ), this, SLOT( OnMessageReceived(ChatMessage &) ));
		QObject::connect((QObject*)contact_, SIGNAL( StateChanged() ), this, SLOT( ContactStateChanged() ));
		QObject::connect(lineEditMessage_, SIGNAL( returnPressed() ), this, SLOT( OnMessageSent() ));
	}

	void Conversation::OnMessageSent()
	{
		QString message(lineEditMessage_->text());
		lineEditMessage_->clear();
		chatSession_->SendTextMessage(message.toStdString());

		QString html("<span style='color:#828282;'>[");
		html.append(GenerateTimeStamp());
		html.append("]</span> <span style='color:#C21511;'>");
		html.append(myName_);
		html.append("</span><span style='color:black;'>: ");
		html.append(message);
		html.append("</span>");
		AppendHTMLToConversation(html);
	}

	void Conversation::ShowMessageHistory(ChatMessageVector messageHistory) 
	{
		ChatMessageVector::const_iterator itrHistory;
		for ( itrHistory = messageHistory.begin(); itrHistory!=messageHistory.end(); itrHistory++ )
		{
			ChatMessage *msg = (*itrHistory);
			QString html("<span style='color:#828282;'>[");
			html.append(msg->GetTimeStamp().toString());
			html.append("]</span> <span style='color:#2133F0;'>");
			html.append(msg->GetAuthor()->GetName().c_str());
			html.append("</span><span style='color:black;'>: ");
			html.append(msg->GetText().c_str());
			html.append("</span>");
			AppendHTMLToConversation(html);
		}
	}

	QString Conversation::GenerateTimeStamp()
	{
		return QTime::currentTime().toString();
	}


	void Conversation::AppendLineToConversation(QString line)
	{
		textEditChat_->appendPlainText(line);
		
	}

	void Conversation::AppendHTMLToConversation(QString html)
	{
		textEditChat_->appendHtml(html);
	}

	void Conversation::OnMessageReceived(ChatMessage &message)
	{
		QString html("<span style='color:#828282;'>[");
		html.append(GenerateTimeStamp());
		html.append("]</span> <span style='color:#2133F0;'>");
		html.append(contact_->GetName().c_str());
		html.append("</span><span style='color:black;'>: ");
		html.append(message.GetText().c_str());
		html.append("</span>");
		AppendHTMLToConversation(html);
	}

	void Conversation::ContactStateChanged()
	{
		// Update status as formatted text to chatwidget
		QString status(contact_->GetPresenceStatus().c_str());
		QString html("<span style='color:#828282;'>[");
		html.append(GenerateTimeStamp());
		html.append("]</span> <span style='color:#828282;'>");
		html.append(contact_->GetName().c_str());
		html.append(" changed status to ");
		html.append(status);
		if ( QString::compare(QString(contact_->GetPresenceMessage().c_str()), QString("")) != 0 )
		{
			html.append(": ");
			html.append(QString(contact_->GetPresenceMessage().c_str()));
		}
		html.append("</span>");
		AppendHTMLToConversation(html);

		// Get icon for the state
		QIcon icon;
		if ( QString::compare(status, QString("available"), Qt::CaseInsensitive) == 0 )
			icon = QIcon(":/images/iconGreen.png");
		else if ( QString::compare(status, QString("offline"), Qt::CaseInsensitive) == 0 )
			icon = QIcon(":/images/iconRed.png");
		else if ( QString::compare(status, QString("away"), Qt::CaseInsensitive) == 0 ||
				  QString::compare(status, QString("hidden"), Qt::CaseInsensitive) == 0 )
			icon = QIcon(":/images/iconOrange.png");
		else if ( QString::compare(status, QString("chat"), Qt::CaseInsensitive) == 0 || 
				  QString::compare(status, QString("dnd"), Qt::CaseInsensitive) == 0 || 
				  QString::compare(status, QString("xa"), Qt::CaseInsensitive) == 0 )
			icon = QIcon(":/images/iconBlue.png");
		else
			icon = QIcon();

		// Set the icon to parent QTabbar with this objects index
		myParent_->setTabIcon(myParent_->indexOf(this), icon);
	}


	/////////////////////////////////////////////////////////////////////
	// CUSTOM QListWidgetItem CLASS
	/////////////////////////////////////////////////////////////////////

	ContactListItem::ContactListItem(QString &name, QString &status, QString &statusmessage, TpQt4Communication::Contact *contact)
		: QListWidgetItem(0, QListWidgetItem::UserType), name_(name), status_(status), statusmessage_(statusmessage), contact_(contact)
	{
		if (QString::compare(statusmessage, QString("")) != 0 )
			status_.append(" - " + statusmessage);
		UpdateItem();
	}

	ContactListItem::~ContactListItem()
	{

	}

	void ContactListItem::UpdateItem()
	{
		// Update status text
		this->setText(name_ + " (" + status_ + ")");
		// Update status icon
		QString status(this->contact_->GetPresenceStatus().c_str());
		if ( QString::compare(status, QString("available"), Qt::CaseInsensitive) == 0 )
			setIcon(QIcon(":/images/iconGreen.png"));
		else if ( QString::compare(status, QString("offline"), Qt::CaseInsensitive) == 0 )
			setIcon(QIcon(":/images/iconRed.png"));
		else if ( QString::compare(status, QString("away"), Qt::CaseInsensitive) == 0 ||
				  QString::compare(status, QString("hidden"), Qt::CaseInsensitive) == 0 )
			setIcon(QIcon(":/images/iconOrange.png"));
		else if ( QString::compare(status, QString("chat"), Qt::CaseInsensitive) == 0 || 
				  QString::compare(status, QString("dnd"), Qt::CaseInsensitive) == 0 || 
				  QString::compare(status, QString("xa"), Qt::CaseInsensitive) == 0 )
			setIcon(QIcon(":/images/iconBlue.png"));
	}

	void ContactListItem::StatusChanged()
	{
		status_ = QString(this->contact_->GetPresenceStatus().c_str());
		if (QString::compare(QString(this->contact_->GetPresenceMessage().c_str()), QString("")) != 0 )
		{
			status_.append(" - ");
			status_.append(this->contact_->GetPresenceMessage().c_str());
		}
		UpdateItem();
	}

	/////////////////////////////////////////////////////////////////////
	// FRIEND REQUEST CLASS
	/////////////////////////////////////////////////////////////////////

	FriendRequestUI::FriendRequestUI(QWidget *parent, FriendRequest *request)
		: QWidget(parent), request_(request), connection_(0)
	{
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
		QUiLoader loader;
        QFile uiFile("./data/ui/communications_friendRequest.ui");
		internalWidget_ = loader.load(&uiFile, this);
		this->layout()->addWidget(internalWidget_);
		uiFile.close();

		originator = findChild<QLabel *>("label_requestOriginator");
		originator->setText(QString(request->GetAddressFrom().c_str()));

		accept = findChild<QPushButton *>("pushButton_Accept");
		reject = findChild<QPushButton *>("pushButton_Deny");
		askLater = findChild<QPushButton *>("pushButton_AskLater");

		QObject::connect(accept, SIGNAL( clicked(bool) ), this, SLOT( ButtonHandlerAccept(bool) ));
		QObject::connect(reject, SIGNAL( clicked(bool) ), this, SLOT( ButtonHandlerReject(bool) ));
		QObject::connect(askLater, SIGNAL( clicked(bool) ), this, SLOT( ButtonHandlerCloseWindow(bool) ));
	}

	FriendRequestUI::FriendRequestUI(QWidget *parent, Connection *connection)
		: QWidget(parent), connection_(connection), request_(0)
	{
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
		QUiLoader loader;
        QFile uiFile("./data/ui/communications_addFriend.ui");
		internalWidget_ = loader.load(&uiFile, this);
		this->layout()->addWidget(internalWidget_);
		uiFile.close();

		accept = findChild<QPushButton *>("pushButton_Send");
		reject = findChild<QPushButton *>("pushButton_Cancel");
		account = findChild<QLineEdit *>("lineEdit_Account");
		message = findChild<QLineEdit *>("lineEdit_Message");

		QObject::connect(accept, SIGNAL( clicked(bool) ), this, SLOT( SendFriendRequest(bool) ));
		QObject::connect(reject, SIGNAL( clicked(bool) ), this, SLOT( ButtonHandlerCloseWindow(bool) ));
	}

	FriendRequestUI::~FriendRequestUI()
	{

	}

	// PRIVATE SLOTS

	void FriendRequestUI::ButtonHandlerAccept(bool clicked)
	{
		request_->Accept();
		emit( CloseThisTab(this) );
	}

	void FriendRequestUI::ButtonHandlerReject(bool clicked)
	{
		request_->Reject();
		emit( CloseThisTab(this) );
	}

	void FriendRequestUI::ButtonHandlerCloseWindow(bool clicked)
	{
		emit( CloseThisTab(this) );
	}

	void FriendRequestUI::SendFriendRequest(bool clicked)
	{
		connection_->SendFriendRequest(account->text().toStdString(), message->text().toStdString());
		emit( CloseThisTab(this) );
	}

} //end if namespace: CommunicationUI