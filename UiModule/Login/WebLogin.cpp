// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Login/WebLogin.h"

#include <QtUiTools>
#include <QFile>
#include <QVBoxLayout>
#include <QPushButton>
#include <QWebFrame>

namespace CoreUi
{
	WebLogin::WebLogin(QWidget *parent, QString address)
		: QWidget(parent), 
          address_(address), 
          widget_(0)
	{
		InitWidget();
		ConnectSignals();
	}

	WebLogin::~WebLogin()
	{
        SAFE_DELETE(webView_);
        SAFE_DELETE(progress_bar_);
        SAFE_DELETE(statusLabel);
        SAFE_DELETE(combobox_address_);
		SAFE_DELETE(refresh_button_);
		SAFE_DELETE(back_button_);
		SAFE_DELETE(forward_button_);
		SAFE_DELETE(stop_button_);
		SAFE_DELETE(go_button_);
		SAFE_DELETE(layout_);
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
		    uiFile.close();

		    // Get ui elements we want to update in signal processing
		    progress_bar_ = widget_->findChild<QProgressBar *>("progressBar_Status");
		    progress_bar_->hide();
		    statusLabel = widget_->findChild<QLabel *>("label_Status");

		    // Add webview by hand, the QFormBuilder cannot do this (custom widget)
		    QVBoxLayout *centerLayout = widget_->layout()->findChild<QVBoxLayout *>("verticalLayout_Container");
		    webView_ = new QWebView(widget_);
		    centerLayout->insertWidget(2, webView_);

		    // Get comboBox and set url to it
		    combobox_address_ = widget_->findChild<QComboBox *>("comboBox_Address");
		    combobox_address_->setEditText(address_);

		    // Get buttons and add image to them
		    back_button_ = widget_->findChild<QPushButton *>("pushButton_Back");
		    back_button_->setIcon(QIcon("./data/ui/images/arrow_left_48.png"));
		    back_button_->setIconSize(QSize(20, 20));
		    forward_button_ = widget_->findChild<QPushButton *>("pushButton_Forward");
		    forward_button_->setIcon(QIcon("./data/ui/images/arrow_right_48.png"));
		    forward_button_->setIconSize(QSize(20, 20));
		    stop_button_ = widget_->findChild<QPushButton *>("pushButton_Stop");
		    stop_button_->setIcon(QIcon("./data/ui/images/cross_48.png"));
		    stop_button_->setIconSize(QSize(20, 20));
		    stop_button_->setEnabled(false);
		    refresh_button_ = widget_->findChild<QPushButton *>("pushButton_Refresh");
		    refresh_button_->setIcon(QIcon("./data/ui/images/refresh_48.png"));
		    refresh_button_->setIconSize(QSize(20, 20));
		    go_button_ = widget_->findChild<QPushButton *>("pushButton_Go");
		    go_button_->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
		    go_button_->setIconSize(QSize(20, 20));
    		
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
		QObject::connect(back_button_, SIGNAL( clicked() ), webView_, SLOT( back() ));
		QObject::connect(forward_button_, SIGNAL( clicked() ), webView_, SLOT( forward() ));
		QObject::connect(stop_button_, SIGNAL( clicked() ), webView_, SLOT( stop() ));
		QObject::connect(refresh_button_, SIGNAL( clicked() ), webView_, SLOT( reload() ));
		QObject::connect(go_button_, SIGNAL( clicked(bool) ), this, SLOT( GoToUrl(bool) ));
		// Addressbar
		QObject::connect(combobox_address_->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( GoToUrl() ));
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
		webView_->setUrl(QUrl(combobox_address_->lineEdit()->text()));
	}

	void WebLogin::LoadStarted()
	{
        if (widget_)
        {
		    stop_button_->setEnabled(true);
		    statusLabel->setText("Loading page...");
		    progress_bar_->show();
        }
	}

	void WebLogin::UpdateUi(int progress)
	{
        if (progress_bar_ != 0)
		    progress_bar_->setValue(progress);
	}

	void WebLogin::ProcessPage(bool success)
	{
        if (widget_ && success)
        {
		    // Update GUI
		    stop_button_->setEnabled(false);
		    address_ = webView_->url().toString();
		    combobox_address_->lineEdit()->setText(address_);
		    setWindowTitle(webView_->page()->mainFrame()->title().append(" - realXtend Naali web browser"));
		    if ( combobox_address_->findText(address_, Qt::MatchFixedString) == -1 )
			    combobox_address_->addItem(address_);
		    statusLabel->setText("Done");
		    progress_bar_->hide();

		    // Do actual HTML page processing if this was a login success page...
		    if ( webView_->page()->mainFrame()->title() == "LoginSuccess") 
			    emit( WebLoginInfoRecieved(webView_->page()->mainFrame()) );
        }
	}

}