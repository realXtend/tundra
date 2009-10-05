// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexWebLogin.h"

#include <QtUiTools>
#include <QFile>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWebFrame>

namespace RexLogic
{
	RexWebLogin::RexWebLogin(QWidget *parent, QString address)
		: QWidget(parent), address_(address)
	{
		InitWidget();
		ConnectSignals();
	}

	RexWebLogin::~RexWebLogin()
	{

	}

	void RexWebLogin::showEvent(QShowEvent *showEvent)
	{
		this->webView_->setUrl(QUrl(address_));
		QWidget::showEvent(showEvent);
	}

	void RexWebLogin::InitWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/weblogin.ui");
		this->widget_ = loader.load(&uiFile, this);
		this->widget_->setMinimumSize(750, 550);
		uiFile.close();

		// Get ui elements we want to update in signal processing
		progressBar = this->widget_->findChild<QProgressBar *>("progressBar_Status");
		progressBar->hide();
		statusLabel = this->widget_->findChild<QLabel *>("label_Status");

		// Add webview by hand, the QFormBuilder cannot do this (custom widget)
		QVBoxLayout *centerLayout = this->widget_->layout()->findChild<QVBoxLayout *>("verticalLayout_Container");
		this->webView_ = new QWebView(this->widget_);
		centerLayout->insertWidget(2, this->webView_);

		// Get comboBox and set url to it
		comboBoxAddress = this->widget_->findChild<QComboBox *>("comboBox_Address");
		comboBoxAddress->setEditText(address_);

		// Get buttons and add image to them
		backButton = this->widget_->findChild<QPushButton *>("pushButton_Back");
		backButton->setIcon(QIcon("./data/ui/images/arrow_left_48.png"));
		backButton->setIconSize(QSize(20, 20));
		forwardButton = this->widget_->findChild<QPushButton *>("pushButton_Forward");
		forwardButton->setIcon(QIcon("./data/ui/images/arrow_right_48.png"));
		forwardButton->setIconSize(QSize(20, 20));
		stopButton = this->widget_->findChild<QPushButton *>("pushButton_Stop");
		stopButton->setIcon(QIcon("./data/ui/images/cross_48.png"));
		stopButton->setIconSize(QSize(20, 20));
		stopButton->setEnabled(false);
		refreshButton = this->widget_->findChild<QPushButton *>("pushButton_Refresh");
		refreshButton->setIcon(QIcon("./data/ui/images/refresh_48.png"));
		refreshButton->setIconSize(QSize(20, 20));
		goButton = this->widget_->findChild<QPushButton *>("pushButton_Go");
		goButton->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
		goButton->setIconSize(QSize(20, 20));
		
		// But widget to layout, set layout to this
		this->layout_ = new QVBoxLayout(this);
		this->layout_->setSpacing(0);
		this->layout_->setMargin(0);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
		this->setWindowTitle("realXtend Naali web browser");
		this->setWindowIcon(QIcon("./data/ui/images/globe_48.png"));
	}

	void RexWebLogin::ConnectSignals()
	{
		// Buttons
		QObject::connect(this->backButton, SIGNAL( clicked() ), this->webView_, SLOT( back() ));
		QObject::connect(this->forwardButton, SIGNAL( clicked() ), this->webView_, SLOT( forward() ));
		QObject::connect(this->stopButton, SIGNAL( clicked() ), this->webView_, SLOT( stop() ));
		QObject::connect(this->refreshButton, SIGNAL( clicked() ), this->webView_, SLOT( reload() ));
		QObject::connect(this->goButton, SIGNAL( clicked(bool) ), this, SLOT( GoToUrl(bool) ));
		// Addressbar
		QObject::connect(this->comboBoxAddress->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( GoToUrl() ));
		// Webview
		QObject::connect(this->webView_, SIGNAL( loadStarted() ), this, SLOT( LoadStarted() ));
		QObject::connect(this->webView_, SIGNAL( loadProgress(int) ), this, SLOT( UpdateUi(int) ));
		QObject::connect(this->webView_, SIGNAL( loadFinished(bool) ), this, SLOT( ProcessPage(bool) ));
	}

	void RexWebLogin::GoToUrl()
	{
		GoToUrl(true);
	}

	void RexWebLogin::GoToUrl(bool checked)
	{
		this->webView_->setUrl(QUrl(this->comboBoxAddress->lineEdit()->text()));
	}

	void RexWebLogin::LoadStarted()
	{
		this->stopButton->setEnabled(true);
		this->statusLabel->setText("Loading page...");
		this->progressBar->show();
	}

	void RexWebLogin::UpdateUi(int progress)
	{
		this->progressBar->setValue(progress);
	}

	void RexWebLogin::ProcessPage(bool success)
	{
		// Update GUI
		this->stopButton->setEnabled(false);
		address_ = this->webView_->url().toString();
		this->comboBoxAddress->lineEdit()->setText(address_);
		QString title(this->webView_->page()->mainFrame()->title());
		title.append(" - realXtend Naali web browser");
		this->setWindowTitle(title);
		if ( this->comboBoxAddress->findText(address_, Qt::MatchFixedString) == -1 )
			this->comboBoxAddress->addItem(address_);
		this->statusLabel->setText("Done");
		this->progressBar->hide();
		// Do actual HTML page processing...
		if ( success )
		{
			QString pageTitle = this->webView_->page()->mainFrame()->title();
			if ( pageTitle == "LoginSuccess") 
			{
				QVariant returnValue;
				returnValue = this->webView_->page()->mainFrame()->evaluateJavaScript("ReturnSuccessValue()");
				emit( LoginProcessed(returnValue.toString()) );
				this->close();
			}
		}
	}

}