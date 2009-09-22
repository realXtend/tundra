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
		connecting_ = false;
		this->layout_ = new QVBoxLayout(this);

		loadUserInterface();

		//// Test code...
		//Conversation *mattik = new Conversation(this);
		//mattik->textEditChat_->appendPlainText("[12:45:21] MattiK: Hello!");
		//mattik->textEditChat_->appendPlainText("[12:47:03] You: Well hello to you too sir!");
		//tabWidgetCoversations_->addTab(mattik, "MattiK");
		//tabWidgetCoversations_->addTab(new Conversation(this), "Sempuki");

	}

	QtUI::~QtUI(void)
	{

	}

	void QtUI::loadUserInterface()
	{
		// Remove widget if there is one
		if (widget_ != NULL && layout_->count() > 0)
			layout_->removeWidget(widget_);

		QUiLoader loader;
		if (connecting_)
		{
			// Init normal GUI
			QFile uiFile("./data/ui/communications.ui");
			this->widget_ = loader.load(&uiFile, this);
			this->widget_->setMinimumSize(472, 284);
			uiFile.close();
		} 
		else
		{
			// Init login GUI
			Login *loginWidget_ = new Login(this);
			this->widget_ = loginWidget_->widget_;
			this->widget_->setMinimumSize(440, 135);
			QObject::connect(loginWidget_, SIGNAL( userdataSet(QString, int, QString, QString) ), this, SLOT( connectToServer(QString, int, QString, QString) ));
			this->setWindowTitle("realXtend Communications login");
		}
		
		if (connecting_) 
		{
			// Get widgets
			tabWidgetCoversations_ = this->widget_->findChild<QTabWidget *>("tabWidget_Conversations");
			tabWidgetCoversations_->clear();
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

		this->layout_->setSpacing(0);
		this->layout_->setMargin(6);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
	}

	void QtUI::setAllEnabled(bool enabled)
	{
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
		connecting_ = true;
		loadUserInterface();

		// Connecti to IM server
		credentials.SetProtocol("jabber");
		credentials.SetUserID(username.toStdString());
		credentials.SetPassword(password.toStdString());
		credentials.SetServer(server.toStdString());
		credentials.SetServerPort(port);

		labelUsername_->setText(username);

		commManager_ = CommunicationManager::GetInstance();
		if (commManager_->GetState() == CommunicationManager::STATE_READY)
		{
			im_connection_ = commManager_->OpenConnection(credentials);

			QObject::connect((QObject *)im_connection_, SIGNAL( Connected() ), this, SLOT( connectionEstablished() ));
			QObject::connect((QObject *)im_connection_, SIGNAL( Error(String &) ), this, SLOT( connectionFailed(QString &) ));
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
	}

	void QtUI::connectionFailed(QString &reason)
	{
		connectionStatus_->setText("Connecting to server failed: " + reason);
	}



	/////////////////////////////////////////////////////////////////////
	// LOGIN CLASS
	/////////////////////////////////////////////////////////////////////

	Login::Login(QWidget *parent)
		: QWidget(parent)
	{
		initWidget();
		connectSignals();
		this->show();
	}

	Login::~Login()
	{

	}

	void Login::initWidget()
	{
		// Init widget from .ui file
		QUiLoader loader;
		QFile uiFile("./data/ui/communications_login.ui");
		this->widget_ = loader.load(&uiFile, this);
		uiFile.close();

		// Get GUI elements
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

	Conversation::Conversation(QWidget *parent)
		: QWidget(parent)
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
		this->widget_->setMinimumSize(472, 284);
		uiFile.close();

		textEditChat_ = this->widget_->findChild<QPlainTextEdit *>("textEdit_Chat");

		this->layout_ = new QVBoxLayout(this);
		this->layout_->setSpacing(0);
		this->layout_->setMargin(0);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
	}

} //end if namespace: CommunicationUI