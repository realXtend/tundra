#ifndef incl_LoginWebView_h
#define incl_LoginWebView_h

#include "ui_loginwebview.h"
#include "loadprogress.h"

#include <QtGui>
#include <QWidget>
#include <QGraphicsView>
#include <QNetworkReply>

class LoginWebView 
	: public QWidget
{
	Q_OBJECT

public:
	LoginWebView(QWidget *parent = 0, QGraphicsView *view = 0);
    ~LoginWebView();
    Ui_LoginWebView *ui;
	LoadProgressPanel *loadSplash;
	void loadAutoLoginPage();

public slots:
	void loadStarted();
	void loadInProgress(int progress);
	void loadFinished(bool success);

private:
	void connectSignals();
	QGraphicsView *myview;

};

#endif // incl_LoginWebView_h