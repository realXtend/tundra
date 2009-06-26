#include "loginwebview.h"
#include "cbloginwidget.h"

LoginWebView::LoginWebView(QWidget *parent, QGraphicsView *view)
	: QWidget(0)
{
	ui = new Ui_LoginWebView;
	ui->setupUi(this);
	ui->webView->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
	loadSplash = new LoadProgressPanel(parent);
	loadSplash->hide();
	loadSplash->setGeometry( (parent->size().width() / 2) - 180, (parent->size().height() / 2) - 50, 0, 0 );

	myview = view;
	
	connectSignals();
}

void LoginWebView::connectSignals()
{
	QObject::connect(ui->webView, SIGNAL( loadStarted() ), SLOT( loadStarted() ));
	QObject::connect(ui->webView, SIGNAL( loadProgress(int) ), SLOT( loadInProgress(int) ));
	QObject::connect(ui->webView, SIGNAL( loadFinished(bool) ), SLOT( loadFinished(bool) ));
}

void LoginWebView::loadAutoLoginPage()
{
	ui->webView->setUrl(QUrl("media/cblogin/rexlogin.html"));
}

////////////////////////////////////////////
//             PUBLIC SLOTS               //
////////////////////////////////////////////

void LoginWebView::loadStarted()
{
	loadSplash->ui->progressBar->setValue(0);
	loadSplash->show();
	myview->setForegroundBrush(QColor(255, 255, 255, 210));
}

void LoginWebView::loadInProgress(int progress) 
{
	loadSplash->ui->progressBar->setValue(progress);
}

void LoginWebView::loadFinished(bool success)
{
	loadSplash->hide();
	myview->setForegroundBrush(QColor(255, 255, 255, 0));
}