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
		initWidget();
		setAllEnabled(true); // False later until succesfull connection

		// Test code...
		Conversation *mattik = new Conversation(this);
		mattik->textEditChat_->appendPlainText("[12:45:21] MattiK: Hello!");
		mattik->textEditChat_->appendPlainText("[12:47:03] You: Well hello to you too sir!");
		tabWidgetCoversations_->addTab(mattik, "MattiK");
		tabWidgetCoversations_->addTab(new Conversation(this), "Sempuki");
	}

	QtUI::~QtUI(void)
	{
	}

	void QtUI::initWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/communications.ui");
		this->widget_ = loader.load(&uiFile, this);
		this->widget_->setMinimumSize(472, 284);
		uiFile.close();

		// Get widgets
		tabWidgetCoversations_ = this->widget_->findChild<QTabWidget *>("tabWidget_Conversations");
		tabWidgetCoversations_->clear();
		buttonSendMessage_ = this->widget_->findChild<QPushButton *>("pushButton_Send");
		buttonSendMessage_->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
		buttonSendMessage_->setIconSize(QSize(25,23));
		lineEditMessage_ = this->widget_->findChild<QLineEdit *>("lineEdit_Message");
		lineEditMessage_->setFocus(Qt::NoFocusReason);
		labelUsername_ = this->widget_->findChild<QLabel *>("label_UserName");
		// SET username, statusmessage and status WHEN YOU GET IT
		labelUsername_->setText("MyUserName@jabber.com");
		lineEditStatus_ = this->widget_->findChild<QLineEdit *>("lineEdit_Status");
		lineEditStatus_->setText("Set custom status click here...");
		comboBoxStatus_ = this->widget_->findChild<QComboBox *>("comboBox_Status");
		// Set connection state from inparams
		connectionStatus_ = this->widget_->findChild<QLabel *>("label_ConnectionStatus");
		connectionStatus_->setText("Connected... (not really)");

		this->layout_ = new QVBoxLayout(this);
		this->layout_->setSpacing(0);
		this->layout_->setMargin(6);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
		this->setWindowTitle("realXtend Communications");
	}

	void QtUI::setAllEnabled(bool enabled)
	{
		buttonSendMessage_->setEnabled(enabled);
		lineEditMessage_->setEnabled(enabled);
		labelUsername_->setEnabled(enabled);
		lineEditStatus_->setEnabled(enabled);
		comboBoxStatus_->setEnabled(enabled);
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