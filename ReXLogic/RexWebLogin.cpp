// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "RexWebLogin.h"

#include <QtUiTools>
#include <QFile>
#include <QVBoxLayout>
#include <QPushButton>

namespace RexLogic
{
	RexWebLogin::RexWebLogin(QWidget *parent, QString address)
		: QWidget(parent), address_(address)
	{
		initWidget();
		connectSignals();
	}

	RexWebLogin::~RexWebLogin()
	{

	}

	void RexWebLogin::showEvent(QShowEvent *showEvent)
	{
		this->webView_->setUrl(QUrl(address_));
		QWidget::showEvent(showEvent);
	}

	void RexWebLogin::initWidget()
	{
		// Load ui to widget from file
		QUiLoader loader;
        QFile uiFile("./data/ui/weblogin.ui");
		this->widget_ = loader.load(&uiFile);
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

		// Add image to button
		goButton = this->widget_->findChild<QPushButton *>("pushButton_Go");
		goButton->setIcon(QIcon("./data/ui/images/arrow_right_green_48.png"));
		goButton->setIconSize(QSize(20, 20));
		
		// But widget to layout, set layout to this
		this->layout_ = new QVBoxLayout(0);
		this->layout_->setSpacing(0);
		this->layout_->setMargin(0);
		this->layout_->addWidget(this->widget_);
		this->setLayout(this->layout_);
	}

	void RexWebLogin::connectSignals()
	{
		QObject::connect(this->comboBoxAddress->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( goToUrl() ));
		QObject::connect(this->goButton, SIGNAL( clicked(bool) ), this, SLOT( goToUrl(bool) ));
		QObject::connect(this->webView_, SIGNAL( loadStarted() ), this, SLOT( loadStarted() ));
		QObject::connect(this->webView_, SIGNAL( loadProgress(int) ), this, SLOT( updateUi(int) ));
		QObject::connect(this->webView_, SIGNAL( loadFinished(bool) ), this, SLOT( processPage(bool) ));
	}

	void RexWebLogin::goToUrl()
	{
		goToUrl(true);
	}

	void RexWebLogin::goToUrl(bool checked)
	{
		this->webView_->setUrl(QUrl(this->comboBoxAddress->lineEdit()->text()));
	}

	void RexWebLogin::loadStarted()
	{
		this->statusLabel->setText("Loading page...");
		this->progressBar->show();
	}

	void RexWebLogin::updateUi(int progress)
	{
		this->progressBar->setValue(progress);
	}

	void RexWebLogin::processPage(bool success)
	{
		address_ = this->webView_->url().toString();
		this->comboBoxAddress->lineEdit()->setText(address_);
		if ( this->comboBoxAddress->findText(address_, Qt::MatchFixedString) == -1 )
			this->comboBoxAddress->addItem(address_);
		this->statusLabel->setText("Done");
		this->progressBar->hide();

		// Do actual HTML page processing...
	}
	

}