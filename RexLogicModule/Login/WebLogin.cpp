// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Login/WebLogin.h"

#include <QtUiTools>
#include <QFile>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWebFrame>

namespace RexLogic
{
	WebLogin::WebLogin(QWidget *parent, QString address)
		: QWidget(parent), address_(address), widget_(0)
	{
		InitWidget();
		ConnectSignals();
	}

	WebLogin::~WebLogin()
	{
        SAFE_DELETE(progressBar);
        SAFE_DELETE(statusLabel);
        SAFE_DELETE(comboBoxAddress);
		SAFE_DELETE(refreshButton);
		SAFE_DELETE(backButton);
		SAFE_DELETE(forwardButton);
		SAFE_DELETE(stopButton);
		SAFE_DELETE(goButton);
		SAFE_DELETE(layout_);
        SAFE_DELETE(webView_);
        SAFE_DELETE(widget_);
	}

	void WebLogin::InitWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/login/login_web.ui");

        if ( uiFile.exists() )
        {
		    widget_ = loader.load(&uiFile, this);
		    widget_->setMinimumSize(750, 550);
		    uiFile.close();

		    // Get ui elements we want to update in signal processing
		    progressBar = widget_->findChild<QProgressBar *>("progressBar_Status");
		    progressBar->hide();
		    statusLabel = widget_->findChild<QLabel *>("label_Status");

		    // Add webview by hand, the QFormBuilder cannot do this (custom widget)
		    QVBoxLayout *centerLayout = widget_->layout()->findChild<QVBoxLayout *>("verticalLayout_Container");
		    webView_ = new QWebView(widget_);
		    webView_->setMinimumSize(100,100);
		    centerLayout->insertWidget(2, webView_);

		    // Get comboBox and set url to it
		    comboBoxAddress = widget_->findChild<QComboBox *>("comboBox_Address");
		    comboBoxAddress->setEditText(address_);

		    // Get buttons and add image to them
		    backButton = widget_->findChild<QPushButton *>("pushButton_Back");
		    backButton->setIcon(QIcon("./data/ui/images/arrow_left_48.png"));
		    backButton->setIconSize(QSize(20, 20));
		    forwardButton = widget_->findChild<QPushButton *>("pushButton_Forward");
		    forwardButton->setIcon(QIcon("./data/ui/images/arrow_right_48.png"));
		    forwardButton->setIconSize(QSize(20, 20));
		    stopButton = widget_->findChild<QPushButton *>("pushButton_Stop");
		    stopButton->setIcon(QIcon("./data/ui/images/cross_48.png"));
		    stopButton->setIconSize(QSize(20, 20));
		    stopButton->setEnabled(false);
		    refreshButton = widget_->findChild<QPushButton *>("pushButton_Refresh");
		    refreshButton->setIcon(QIcon("./data/ui/images/refresh_48.png"));
		    refreshButton->setIconSize(QSize(20, 20));
		    goButton = widget_->findChild<QPushButton *>("pushButton_Go");
		    goButton->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
		    goButton->setIconSize(QSize(20, 20));
    		
		    // But widget to layout, set layout to this
		    layout_ = new QVBoxLayout(this);
		    layout_->setSpacing(0);
		    layout_->setMargin(0);
		    layout_->addWidget(widget_);
		    setLayout(layout_);
		    setWindowTitle("realXtend Naali web browser");
		    setWindowIcon(QIcon("./data/ui/images/globe_48.png"));
        }
	}

	void WebLogin::ConnectSignals()
	{
		// Buttons
		QObject::connect(backButton, SIGNAL( clicked() ), webView_, SLOT( back() ));
		QObject::connect(forwardButton, SIGNAL( clicked() ), webView_, SLOT( forward() ));
		QObject::connect(stopButton, SIGNAL( clicked() ), webView_, SLOT( stop() ));
		QObject::connect(refreshButton, SIGNAL( clicked() ), webView_, SLOT( reload() ));
		QObject::connect(goButton, SIGNAL( clicked(bool) ), this, SLOT( GoToUrl(bool) ));
		// Addressbar
		QObject::connect(comboBoxAddress->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( GoToUrl() ));
		// Webview
		QObject::connect(webView_, SIGNAL( loadStarted() ), this, SLOT( LoadStarted() ));
		QObject::connect(webView_, SIGNAL( loadProgress(int) ), this, SLOT( UpdateUi(int) ));
		QObject::connect(webView_, SIGNAL( loadFinished(bool) ), this, SLOT( ProcessPage(bool) ));

        webView_->setUrl(QUrl(address_));
	}

	void WebLogin::GoToUrl()
	{
		GoToUrl(true);
	}

	void WebLogin::GoToUrl(bool checked)
	{
		webView_->setUrl(QUrl(comboBoxAddress->lineEdit()->text()));
	}

	void WebLogin::LoadStarted()
	{
        if (widget_)
        {
		    stopButton->setEnabled(true);
		    statusLabel->setText("Loading page...");
		    progressBar->show();
        }
	}

	void WebLogin::UpdateUi(int progress)
	{
        if (progressBar != 0)
		    progressBar->setValue(progress);
	}

	void WebLogin::ProcessPage(bool success)
	{
        if (widget_)
        {
		    // Update GUI
		    stopButton->setEnabled(false);
		    address_ = webView_->url().toString();
		    comboBoxAddress->lineEdit()->setText(address_);
		    QString title(webView_->page()->mainFrame()->title());
		    title.append(" - realXtend Naali web browser");
		    setWindowTitle(title);
		    if ( comboBoxAddress->findText(address_, Qt::MatchFixedString) == -1 )
			    comboBoxAddress->addItem(address_);
		    statusLabel->setText("Done");
		    progressBar->hide();
		    // Do actual HTML page processing...
		    if ( success )
		    {
			    QString pageTitle = webView_->page()->mainFrame()->title();
			    if ( pageTitle == "LoginSuccess") 
				    emit( WebLoginInfoRecieved(webView_->page()->mainFrame()) );
		    }
        }
	}

}