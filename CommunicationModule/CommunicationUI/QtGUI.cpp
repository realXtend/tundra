#include <StableHeaders.h>
#include <QtModule.h>
#include <UICanvasManager.h>
#include <QtUiTools>
#include <QFile>
#include <QTime>
#include "QtGUI.h"

namespace CommunicationUI
{

	QtGUI::QtGUI(Foundation::Framework *framework)
		: framework_(framework), UIContainer_(0)
	{
		LogInfo("Loading UIController to QtModule UICanvas...");
		// Get comm manager to check state
		communication_service_ = Communication::CommunicationService::GetInstance();
		if (communication_service_ == 0)
		{
			throw Core::Exception("Cannot get CommunicationService object.");
		}
		// Use QtModule to show our QtUI widget
		boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
		
		if ( qt_module.get() == 0)
		{	
			LogWarning("Could not aqquire QtModule and show Comm UI");
		}
		else 
		{
			// Set param to QtUI::UICanvas::Internal to put inside ogre window
			canvas_login_ = qt_module->CreateCanvas(UICanvas::Internal).lock();
            canvas_login_->Hide();
            canvas_chat_ = qt_module->CreateCanvas(UICanvas::Internal).lock();
            canvas_chat_->Hide();

			UIContainer_ = new UIContainer(0, framework_, canvas_login_, canvas_chat_);

			// Connect signal for resizing
			QObject::connect(UIContainer_, SIGNAL( ChangeToolBarButton(QString, QString) ), this, SLOT( ChangeToolBarButton(QString, QString) ));
			QObject::connect(UIContainer_, SIGNAL( DestroyCanvas() ), this, SLOT( DestroyThis() ));

			// Add to control bar
			qt_module->AddCanvasToControlBar(canvas_login_->GetID(), QString("IM Login"));
			LogInfo("Loading succesfull");
		}		
	}

	QtGUI::~QtGUI(void)
	{
		DestroyThis();
	}

	void QtGUI::DestroyThis()
	{
		boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
		if (qt_module.get())
		{
            if (canvas_login_)
			    qt_module->DeleteCanvas(canvas_login_->GetID());
            if (canvas_chat_)
                qt_module->DeleteCanvas(canvas_chat_->GetID());
		}

        if (UIContainer_)
            SAFE_DELETE(UIContainer_);
	}

	void QtGUI::ChangeToolBarButton(QString oldID, QString newID)
	{
        boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
		if (qt_module.get())
        {
            qt_module->RemoveCanvasFromControlBar(oldID);
            qt_module->AddCanvasToControlBar(newID, QString("IM"));
        }
	}

	/////////////////////////////////////////////////////////////////////
	// UIContainer CLASS
	/////////////////////////////////////////////////////////////////////

    UIContainer::UIContainer(QWidget *parent, Foundation::Framework *framework, boost::shared_ptr<UICanvas> canvas_login, boost::shared_ptr<UICanvas> canvas_chat)
		: QWidget(parent), chatWidget_(0), loginWidget_(0), framework_(framework), canvas_login_(canvas_login), canvas_chat_(canvas_chat), labelLoginConnectionStatus_(0), popup_(0), notify_timer_(0)
	{
		communication_service_ = Communication::CommunicationService::GetInstance();

		LogInfo("Creating Login UI");
		this->setLayout(new QVBoxLayout(this));
		this->layout()->setMargin(0);
		this->setObjectName(QString("containerWidget"));
        this->setStyleSheet("QWidget#containerWidget { background-color: rgba(0,0,0,0); padding: 0px; margin: 0px; }");
		LoadUserInterface(false);
	}

	UIContainer::~UIContainer(void)
	{
		delete layout();
        if (chatWidget_)
        {
            SAFE_DELETE(tabWidgetCoversations_);
            SAFE_DELETE(listWidgetFriends_);
            SAFE_DELETE(labelUsername_);
            SAFE_DELETE(lineEditStatus_);
            SAFE_DELETE(comboBoxStatus_);
            SAFE_DELETE(buttonAddFriend_);
            SAFE_DELETE(buttonRemoveFriend_);
        }
        if (loginWidget_)
        {
            SAFE_DELETE(labelLoginConnectionStatus_);
        }
	}

	void UIContainer::LoadUserInterface(bool connected)
	{
		if (connected && chatWidget_ == 0)
		{
            // Init chat GUI
			QUiLoader loader;
			QFile uiFile("./data/ui/communication/communications.ui");
			chatWidget_ = loader.load(&uiFile, 0);
            chatWidget_->resize(720, 350);
			uiFile.close();

			// Insert custom tab widget
			tabWidgetCoversations_ = new ConversationsContainer(chatWidget_);
			tabWidgetCoversations_->clear();
			QHBoxLayout *friendsAndTabWidget = chatWidget_->findChild<QHBoxLayout *>("horizontalLayout_FriendsAndTabWidget");
			friendsAndTabWidget->insertWidget(1, tabWidgetCoversations_); 
			friendsAndTabWidget->setStretch(1,2);
			
			// Get widgets
			listWidgetFriends_ = chatWidget_->findChild<QListWidget *>("listWidget_Friends");
			listWidgetFriends_->setIconSize(QSize(10,10));
			labelUsername_ = chatWidget_->findChild<QLabel *>("label_UserName");
			lineEditStatus_ = chatWidget_->findChild<QLineEdit *>("lineEdit_Status");
			comboBoxStatus_ = chatWidget_->findChild<QComboBox *>("comboBox_Status");
			comboBoxStatus_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
			comboBoxStatus_->setIconSize(QSize(10,10));
			buttonAddFriend_ = chatWidget_->findChild<QPushButton *>("pushButton_AddFriend");
			buttonAddFriend_->setIcon(QIcon(":/images/iconAdd.png"));
			buttonAddFriend_->setLayoutDirection(Qt::RightToLeft);
			buttonAddFriend_->setIconSize(QSize(10,10));
			buttonRemoveFriend_ = chatWidget_->findChild<QPushButton *>("pushButton_RemoveFriend");
			buttonRemoveFriend_->setIcon(QIcon(":/images/iconRemove.png"));
			buttonRemoveFriend_->setIconSize(QSize(10,10));
            
			canvas_chat_->SetResizable(false);
            canvas_chat_->SetPosition(20,30);
		    canvas_chat_->SetSize(720, 350);
            canvas_chat_->AddWidget(chatWidget_);
            canvas_login_->Hide();
            canvas_chat_->Show();
            emit( ChangeToolBarButton(canvas_login_->GetID(), canvas_chat_->GetID()) );

		} 
		else
		{
            if (loginWidget_ == 0)
			{			
			    // Init login GUI
			    loginWidget_ = new Login(0, currentMessage, framework_);
                loginWidget_->setObjectName(QString("containerMiddleChat"));
                loginWidget_->setStyleSheet("QWidget#containerMiddleChat { background-color: rgba(0,0,0,0); padding: 0px; margin: 0px; }");
			    // Get widgets
			    labelLoginConnectionStatus_ = loginWidget_->findChild<QLabel *>("label_Status");
			    // Connect signals
			    QObject::connect(loginWidget_, SIGNAL( UserdataSet(QString, int, QString, QString) ), this, SLOT( ConnectToServer(QString, int, QString, QString) ));
			    
    			canvas_login_->SetResizable(false);
                canvas_login_->SetPosition(20,30);
			    canvas_login_->SetSize(400, 185);
                canvas_login_->AddWidget(loginWidget_);
            }
		}

	}

	void UIContainer::LoadConnectedUserData(Communication::ConnectionInterface *connection)
	{
		QString user_id = connection->GetUserID();;
		labelUsername_->setText(user_id); 
		
		// Update contacts
		ContactListChanged(connection->GetContacts().GetContacts());

		// Get available state options
		comboBoxStatus_->clear();
		comboBoxStatus_->addItems(im_connection_->GetPresenceStatusOptionsForUser());

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
		// Connect to IM server
		credentials_.SetProtocol("jabber");
		if (!username.contains("@"))
			username.append(QString("@%1").arg(server));
		credentials_.SetUserID(username);
		credentials_.SetPassword(password);
		credentials_.SetServer(server);
		credentials_.SetPort(port);

		try
		{
			im_connection_ = communication_service_->OpenConnection(credentials_);
		}
		catch (Core::Exception &e)
		{
			QString error_message = QString("Error: ").append(e.what());
			labelLoginConnectionStatus_->setText(error_message);
			this->LoadUserInterface(false);
			return;
		}

		labelLoginConnectionStatus_->setText("Connecting...");
		connect((QObject*)im_connection_, SIGNAL( ConnectionReady(Communication::ConnectionInterface&) ), this, SLOT( ConnectionEstablished(Communication::ConnectionInterface&) ));
		connect((QObject*)im_connection_, SIGNAL( ConnectionError(Communication::ConnectionInterface&) ), this, SLOT( ConnectionFailed(Communication::ConnectionInterface&) ));
	}

	void UIContainer::ManagerReady()
	{
	}

	void UIContainer::ConnectionEstablished(Communication::ConnectionInterface &connection)
	{
		if (loginWidget_ != 0)
			((Login *)loginWidget_)->SaveConfig();

		labelLoginConnectionStatus_->setText("Connected");
		
		if ( im_connection_ != NULL )
		{
			connect(im_connection_, SIGNAL( ChatSessionReceived(Communication::ChatSessionInterface& ) ), this, SLOT( NewChatSessionRequest(Communication::ChatSessionInterface&) ));
			connect(im_connection_, SIGNAL( FriendRequestReceived(Communication::FriendRequestInterface&) ), this, SLOT( NewFriendRequest(Communication::FriendRequestInterface&) ));
            connect(im_connection_, SIGNAL( NewContact(const Communication::ContactInterface&) ), this, SLOT( OnNewContact(const Communication::ContactInterface&) ));
            connect(im_connection_, SIGNAL( ContactRemoved(const Communication::ContactInterface&) ), this, SLOT( OnContactRemoved(const Communication::ContactInterface&) ));
			LoadUserInterface(true);
			LoadConnectedUserData(im_connection_);
		}
		else
		{
			ConnectionFailed(*im_connection_);
		}
	}

	void UIContainer::ConnectionFailed(Communication::ConnectionInterface &connection)
	{
		QString message = QString("Connection failed: ").append(connection.GetReason());
		LogDebug(message.toStdString());
		this->currentMessage = message;
		this->LoadUserInterface(false);
	}

	void UIContainer::ContactListChanged(Communication::ContactVector contacts)
	{
		listWidgetFriends_->clear();
		Communication::ContactVector::const_iterator itrContacts;

		for( itrContacts=contacts.begin(); itrContacts!=contacts.end(); itrContacts++ )
		{
			Communication::ContactInterface *contact = (*itrContacts);
		        QString pstatus = contact->GetPresenceStatus(), pmessage = contact -> GetPresenceMessage(),
                                cname = contact->GetName();
			ContactListItem *contactItem = new ContactListItem(cname, pstatus, pmessage, contact);
			connect(contact, SIGNAL( PresenceStatusChanged(const QString &, const QString &) ), contactItem, SLOT( StatusChanged(const QString &, const QString &) ));
			listWidgetFriends_->addItem(contactItem);
		}
	}

	// GUI RELATED SLOTS //

	void UIContainer::StatusChanged(const QString &newStatus)
	{
		im_connection_->SetPresenceStatus(newStatus);
	}

	void UIContainer::StatusMessageChanged()
	{
		im_connection_->SetPresenceMessage(lineEditStatus_->text());
	}

	void UIContainer::StartNewChat(QListWidgetItem *clickedItem)
	{
		ContactListItem *listItem = (ContactListItem *)clickedItem;
		if ( tabWidgetCoversations_->DoesTabExist(listItem->contact_) == false )
		{
			Communication::ChatSessionInterface* chatSession = im_connection_->OpenPrivateChatSession(*listItem->contact_);
			QString user_name = im_connection_->GetUserID();
			Conversation *conversation = new Conversation(tabWidgetCoversations_, *chatSession, listItem->contact_, user_name);
			tabWidgetCoversations_->addTab(conversation, clickedItem->icon(), QString(listItem->contact_->GetName()));
			tabWidgetCoversations_->setCurrentWidget(conversation);
            // For popup
            connect((QObject*)chatSession, SIGNAL( MessageReceived(const Communication::ChatMessageInterface &) ), this, SLOT( NewMessageReceived(const Communication::ChatMessageInterface &) ));
		}
	}

	void UIContainer::NewChatSessionRequest(Communication::ChatSessionInterface &chatSession)
	{
		Communication::ChatSessionParticipantVector participants = chatSession.GetParticipants();
		if (participants.size() == 0)
			return; //! @todo throw exception or something

		Communication::ChatSessionParticipantInterface* participant = participants[0];
		Communication::ContactInterface* originator = participant->GetContact();
		QString originator_name = originator->GetName();
		QString originator_presence_status = originator->GetPresenceStatus();

		QString user_name = im_connection_->GetUserID();
		if (originator_name.size() > 0  )
		{
			if ( tabWidgetCoversations_->DoesTabExist(originator) == false )
			{
				Conversation *conversation = new Conversation(tabWidgetCoversations_, chatSession, originator, user_name);
				conversation->ShowMessageHistory(chatSession.GetMessageHistory());
                NewMessageReceived(*chatSession.GetMessageHistory().back());
				tabWidgetCoversations_->addTab(conversation, GetStatusIcon(originator_presence_status), originator_name);		
				tabWidgetCoversations_->setCurrentWidget(conversation);
                // For popup
                connect((QObject*)&chatSession, SIGNAL( MessageReceived(const Communication::ChatMessageInterface &) ), this, SLOT( NewMessageReceived(const Communication::ChatMessageInterface &) ));
			}
		}
		else
		{
			LogWarning("NewChatSessionRequest (handled successfully)");
		}
	}

	void UIContainer::NewFriendRequest(Communication::FriendRequestInterface &request)
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
		LogDebug("RemoveFriend clicked");
        QListWidgetItem* item = listWidgetFriends_->currentItem();
        if (item == 0)
            return;

        QString contact_id = item->text();
        ContactListItem* contact_item =  (ContactListItem *)(item);
        if (contact_item == 0)
            return;
        im_connection_->RemoveContact( *contact_item->GetContact() );
        // The friend list widget will be upted when it get ContactRemoved signal from Connection object
	}

    void UIContainer::OnNewContact(const Communication::ContactInterface& contact)
    {
        ContactListChanged(im_connection_->GetContacts().GetContacts());
    }

    void UIContainer::OnContactRemoved(const Communication::ContactInterface& contact)
    {
        ContactListChanged(im_connection_->GetContacts().GetContacts());
    }

	void UIContainer::closeEvent(QCloseEvent *myCloseEvent) 
	{
		LogInfo("CloseEvent catched");
		emit ( DestroyCanvas() );
		QWidget::closeEvent(myCloseEvent);
	}

    void UIContainer::NewMessageReceived(const Communication::ChatMessageInterface &message)
    {
        if (canvas_chat_->IsHidden())
        {
            boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
            if (qt_module.get())
            {
                if (notify_timer_)
                {
                    notify_timer_->stop();
                    SAFE_DELETE(notify_timer_);
                }
                if (canvas_notify_popup_.get())
                {
                    canvas_notify_popup_->Hide();
                    qt_module->DeleteCanvas(canvas_notify_popup_->GetID());
                }

                canvas_notify_popup_ = qt_module->CreateCanvas(QtUI::UICanvas::Internal).lock();

                QUiLoader loader;
                QFile uiFile("./data/ui/communication/notify_popup.ui");

                if ( uiFile.exists() )
                {
                    // ui init
                    popup_ = loader.load(&uiFile);
                    notify_title_ = popup_->findChild<QLabel *>("titleLabel");
                    notify_message_ = popup_->findChild<QLabel *>("messageLabel");
                    notify_autohide_ = popup_->findChild<QLabel *>("autohideLabel");
                    notify_show_button_ = popup_->findChild<QPushButton *>("showButton");
                    
                    uiFile.close();

                    // content to ui
                    notify_title_->setText(QString("You have recieved a message from %1").arg(message.GetOriginator()->GetContact()->GetName()));
                    notify_message_->setText(QString("%1").arg(message.GetText()));
                    notify_autohide_->setText(QString("Autohide in 10"));
                    // timer and store tab name
                    QObject::connect(notify_show_button_, SIGNAL( clicked() ), this, SLOT( ShowUiAndNewMessage() ));
                    tabname_ = message.GetOriginator()->GetContact()->GetName();
                    
                    notify_timer_ = new QTimer();
                    QObject::connect(notify_timer_, SIGNAL( timeout() ), this, SLOT( NotifyTimerUpdate() ));
                    timer_count_ = 10;
                    notify_timer_->start(1000);

                    // sizing and show
                    popup_->resize(408, 83);
                    canvas_notify_popup_->SetSize(408, 83);
                    canvas_notify_popup_->SetPosition(0, 25);
                    canvas_notify_popup_->SetResizable(false);
                    canvas_notify_popup_->SetStationary(true);
                    canvas_notify_popup_->SetAlwaysOnTop(true);
                    canvas_notify_popup_->AddWidget(popup_);
                    canvas_notify_popup_->Show();
                    canvas_notify_popup_->BringToTop();
                }
            }
        }
    }

    void UIContainer::ShowUiAndNewMessage()
    {
        notify_timer_->stop();
        tabWidgetCoversations_->SetFocusTo(tabname_);
        canvas_chat_->Show();
        
        boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
        if (qt_module.get() && canvas_notify_popup_.get())
        {
            canvas_notify_popup_->Hide();
            qt_module->DeleteCanvas(canvas_notify_popup_->GetID());
        }
    }

    void UIContainer::NotifyTimerUpdate()
    {
        if (canvas_notify_popup_ && notify_autohide_ && popup_ && notify_timer_)
        {
            if (timer_count_ > 0)
            {
                notify_autohide_->setText(QString("Autohide in %1").arg(QString::number(timer_count_)));
                --timer_count_;
            }
            else
            {
                boost::shared_ptr<QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtModule>(Foundation::Module::MT_Gui).lock();
                if (qt_module.get() && canvas_notify_popup_.get())
                {
                    notify_timer_->stop();
                    canvas_notify_popup_->Hide();
                    qt_module->DeleteCanvas(canvas_notify_popup_->GetID());
                    canvas_notify_popup_.reset();    
                }
            }
        }

    }


	/////////////////////////////////////////////////////////////////////
	// LOGIN CLASS
	/////////////////////////////////////////////////////////////////////

	Login::Login(QWidget *parent, QString &message, Foundation::Framework *framework)
		: QWidget(parent), framework_(framework)
	{
		this->setLayout(new QVBoxLayout(this));
		this->layout()->setMargin(0);
		InitWidget(message);
        ReadConfig();
		ConnectSignals();
	}

	Login::~Login()
	{
        SAFE_DELETE(textEditServer_);
        SAFE_DELETE(textEditPort_);
        SAFE_DELETE(textEditUsername_);
        SAFE_DELETE(textEditPassword_);
        SAFE_DELETE(buttonConnect_);
        SAFE_DELETE(buttonCancel_);
        SAFE_DELETE(internalWidget_);
        delete layout();
	}

	void Login::InitWidget(QString &message)
	{
		// Init widget from .ui file
		QUiLoader loader;
		QFile uiFile("./data/ui/communication/communications_login.ui");
		internalWidget_ = loader.load(&uiFile, this);
		internalWidget_->layout()->setMargin(0);
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
        buttonConnect_->setEnabled(true);
		buttonCancel_ = findChild<QPushButton *>("pushButton_Cancel");
	}

    void Login::ReadConfig()
    {
        QString value, configKey;
        QString server, username, port;
		QString configGroup("InstantMessagingLogin");
        bool ok = false;

		configKey = QString("server");
		server = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
		configKey = QString("username");
		username = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());
		configKey = QString("port");
		port = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(configGroup.toStdString(), configKey.toStdString()).c_str());

        if (!server.isNull() && !server.isEmpty())
            textEditServer_->setText(server);
        if (!username.isNull() && !username.isEmpty())
            textEditUsername_->setText(username);
        port.toInt(&ok);
        if (ok)
            textEditPort_->setText(port);
        
    }

    void Login::SaveConfig()
    {
        // Save config
        framework_->GetConfigManager()->SetSetting<std::string>(std::string("InstantMessagingLogin"), std::string("server"), textEditServer_->text().toStdString());
        framework_->GetConfigManager()->SetSetting<std::string>(std::string("InstantMessagingLogin"), std::string("username"), textEditUsername_->text().toStdString());
        framework_->GetConfigManager()->SetSetting<std::string>(std::string("InstantMessagingLogin"), std::string("port"), textEditPort_->text().toStdString());
        framework_->GetConfigManager()->Export();
    }

	void Login::ConnectSignals()
	{
		QObject::connect(buttonConnect_, SIGNAL( clicked(bool) ), this, SLOT ( CheckInput(bool) ));
        QObject::connect(textEditPassword_, SIGNAL( returnPressed() ), this, SLOT ( CheckInput() ));
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
            buttonConnect_->setText("Connecting");
            buttonConnect_->setEnabled(false);
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
        // go trough count() and delete widgets
	}

	void ConversationsContainer::CloseFriendRequest(FriendRequestUI *request)
	{
		this->removeTab(this->indexOf(request));
        //! @todo Check if here is memory leak.
        //SAFE_DELETE(request);
	}

	void ConversationsContainer::CloseTab(int index)
	{
		QWidget *child = this->widget(index);
		Conversation *childConversation = dynamic_cast<Conversation *>(child);
		if (childConversation != 0)
			childConversation->chat_session_.Close();
		this->removeTab(index);
		delete child;
		child = 0;
	}

	bool ConversationsContainer::DoesTabExist(Communication::ContactInterface *contact)
	{
		for (int i=0; i<this->count(); i++)
		{
			if ( QString::compare(this->tabText(i), contact->GetName()) == 0 )
			{
				this->setCurrentIndex(i);
				return true;
			}
		}
		return false;
	}

    bool ConversationsContainer::SetFocusTo(QString &tabName)
	{
		for (int i=0; i<this->count(); i++)
		{
			if ( QString::compare(this->tabText(i), tabName) == 0 )
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

	Conversation::Conversation(ConversationsContainer *parent, Communication::ChatSessionInterface &chatSession, Communication::ContactInterface *contact, QString name)
		: QWidget(parent), myParent_(parent), chat_session_(chatSession), contact_(contact), myName_(name)
	{
		this->setLayout(new QVBoxLayout(this));
		this->layout()->setMargin(0);
		InitWidget();
		ConnectSignals();
	}

	Conversation::~Conversation()
	{
        delete layout();
        SAFE_DELETE(internalWidget_);
	}

	void Conversation::InitWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/communication/communications_conversation.ui");
		internalWidget_ = loader.load(&uiFile, this);
		this->layout()->addWidget(internalWidget_);
		uiFile.close();
		
		textEditChat_ = findChild<QPlainTextEdit *>("textEdit_Chat");
		lineEditMessage_ = findChild<QLineEdit *>("lineEdit_Message");
		lineEditMessage_->setFocus(Qt::NoFocusReason);

		QString startMessage("Started chat session with ");
		startMessage.append(contact_->GetName());
		startMessage.append("...");
		AppendLineToConversation(startMessage);
	}

	void Conversation::ConnectSignals()
	{
		connect(&chat_session_, SIGNAL( MessageReceived(const Communication::ChatMessageInterface &) ), this, SLOT( OnMessageReceived(const Communication::ChatMessageInterface &) ));
		connect(contact_, SIGNAL( PresenceStatusChanged(const QString &, const QString &) ), this, SLOT( ContactStateChanged(const QString &, const QString &) ));
		connect(lineEditMessage_, SIGNAL( returnPressed() ), this, SLOT( OnMessageSent() ));
	}

	void Conversation::OnMessageSent()
	{
		QString message(lineEditMessage_->text());
		lineEditMessage_->clear();
		chat_session_.SendMessage(message);

		QString html("<span style='color:#828282;'>[");
		html.append(GenerateTimeStamp());
		html.append("]</span> <span style='color:#C21511;'>");
		html.append(myName_);
		html.append("</span><span style='color:black;'>: ");
		html.append(message);
		html.append("</span>");
		AppendHTMLToConversation(html);
	}

	void Conversation::ShowMessageHistory(Communication::ChatMessageVector messageHistory) 
	{
		Communication::ChatMessageVector::const_iterator itrHistory;
		for ( itrHistory = messageHistory.begin(); itrHistory!=messageHistory.end(); itrHistory++ )
		{
			Communication::ChatMessageInterface *msg = (*itrHistory);
			QString html("<span style='color:#828282;'>[");
			html.append(GenerateTimeStamp(msg->GetTimeStamp()));
			html.append("]</span> <span style='color:#2133F0;'>");
			html.append(msg->GetOriginator()->GetName());
			//! @todo check if the originator is the current user
			html.append("</span><span style='color:black;'>: ");
			html.append(msg->GetText());
			html.append("</span>");
			AppendHTMLToConversation(html);
		}
	}

	QString Conversation::GenerateTimeStamp()
	{
        QDateTime time_stamp = QDateTime::currentDateTime();

        QString timestamp(QString("%1 %2").arg(time_stamp.date().toString("dd.MM.yyyy"),time_stamp.time().toString("hh:mm:ss")));
		return timestamp;
	}

    QString Conversation::GenerateTimeStamp(QDateTime time)
    {
        QString timestamp(QString("%1 %2").arg(time.date().toString("dd.MM.yyyy"),time.time().toString("hh:mm:ss")));
		return timestamp;
    }

	void Conversation::AppendLineToConversation(QString line)
	{
		textEditChat_->appendPlainText(line);
	}

	void Conversation::AppendHTMLToConversation(QString html)
	{
		textEditChat_->appendHtml(html);
	}

	void Conversation::OnMessageReceived(const Communication::ChatMessageInterface &message)
	{
        QString timestamp(QString("%1 %2").arg(message.GetTimeStamp().date().toString("dd.MM.yyyy"),message.GetTimeStamp().time().toString("hh:mm:ss")));
		QString html("<span style='color:#828282;'>[");
		html.append(timestamp);
		html.append("]</span> <span style='color:#2133F0;'>");
		html.append(contact_->GetName());
		html.append("</span><span style='color:black;'>: ");
		html.append(message.GetText());
		html.append("</span>");
		AppendHTMLToConversation(html);
	}

	void Conversation::ContactStateChanged(const QString &status, const QString &message)
	{
		// Update status as formatted text to chatwidget
		QString html("<span style='color:#828282;'>[");
		html.append(GenerateTimeStamp());
		html.append("]</span> <span style='color:#828282;'>");
		html.append(contact_->GetName());
		html.append(" changed status to ");
		html.append(status);
		if ( message.size() > 0 )
		{
			html.append(": ");
			html.append(message);
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

	ContactListItem::ContactListItem(QString &name, QString &status, QString &statusmessage, Communication::ContactInterface *contact)
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
		QString status(this->contact_->GetPresenceStatus());
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

	void ContactListItem::StatusChanged(const QString &status, const QString &message)
	{
		status_ = QString(this->contact_->GetPresenceStatus());
		if (QString::compare(this->contact_->GetPresenceMessage(), QString("")) != 0 )
		{
			status_.append(" - ");
			status_.append(this->contact_->GetPresenceMessage());
		}
		UpdateItem();
	}

	/////////////////////////////////////////////////////////////////////
	// FRIEND REQUEST CLASS
	/////////////////////////////////////////////////////////////////////

	FriendRequestUI::FriendRequestUI(QWidget *parent, Communication::FriendRequestInterface &request)
		: QWidget(parent), request_(&request), connection_(0)
	{
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
		QUiLoader loader;
        QFile uiFile("./data/ui/communication/communications_friendRequest.ui");
		internalWidget_ = loader.load(&uiFile, this);
		this->layout()->addWidget(internalWidget_);
		uiFile.close();

		originator = findChild<QLabel *>("label_requestOriginator");
		originator->setText(request.GetOriginatorID());

		accept = findChild<QPushButton *>("pushButton_Accept");
		reject = findChild<QPushButton *>("pushButton_Deny");
		askLater = findChild<QPushButton *>("pushButton_AskLater");

		QObject::connect(accept, SIGNAL( clicked(bool) ), this, SLOT( ButtonHandlerAccept(bool) ));
		QObject::connect(reject, SIGNAL( clicked(bool) ), this, SLOT( ButtonHandlerReject(bool) ));
		QObject::connect(askLater, SIGNAL( clicked(bool) ), this, SLOT( ButtonHandlerCloseWindow(bool) ));
	}

	FriendRequestUI::FriendRequestUI(QWidget *parent, Communication::ConnectionInterface *connection)
		: QWidget(parent), connection_(connection), request_(0)
	{
		this->setLayout(new QVBoxLayout());
		this->layout()->setMargin(0);
		QUiLoader loader;
        QFile uiFile("./data/ui/communication/communications_addFriend.ui");
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
        delete layout();
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
		connection_->SendFriendRequest(account->text(), message->text());
		emit( CloseThisTab(this) );
	}

} //end if namespace: CommunicationUI
