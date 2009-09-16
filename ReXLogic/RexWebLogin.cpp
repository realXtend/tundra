// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexWebLogin.h"

#include <QtUiTools>
#include <QFile>
#include <QVBoxLayout>
#include <QPushButton>

namespace RexLogic
{
	RexWebLogin::RexWebLogin(QWidget *parent)
		: QWidget(parent)
	{
		initWidget();
	}

	RexWebLogin::~RexWebLogin()
	{

	}

	void RexWebLogin::initWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/weblogin.ui");
		this->widget_ = loader.load(&uiFile, this);
		this->widget_->setMinimumSize(750, 550);
		uiFile.close();

		// Add webview by hand, the QFormBuilder cannot do this (custom widget)
		QVBoxLayout *centerLayout = this->widget_->layout()->findChild<QVBoxLayout *>("verticalLayout_Container");
		this->webView_ = new QWebView(this);
		this->webView_->setUrl(QUrl("http://www.google.com"));
		centerLayout->insertWidget(2, this->webView_);

		// Add image to button
		QPushButton *goButton = this->widget_->findChild<QPushButton *>("pushButton_Go");
		goButton->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
		goButton->setIconSize(QSize(20, 20));
		
		// But widget to layout, set layout to this
		this->layout_ = new QVBoxLayout(this);
		this->layout_->setSpacing(0);
		this->layout_->setMargin(0);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
	}

	void RexWebLogin::connectSignals()
	{
		// webview finished to progressbar and statuslabel
		// webview to parsehandler after load is done, see RexCBLogin for handling/parsing
	}
}