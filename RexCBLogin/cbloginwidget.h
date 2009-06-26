#ifndef CBLOGINWIDGET_H
#define CBLOGINWIDGET_H

#include "ui_cbloginframe.h"

#include "customproxy.h"
#include "loginpanel.h"
#include "loginwebview.h"
#include "loadprogress.h"
#include "configmanager.h"

#include <QtGui>
#include <QBrush>
#include <QWebFrame>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsLinearLayout>

class CBLoginWidget 
	: public QFrame
{
	Q_OBJECT

public:
	CBLoginWidget(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CBLoginWidget();

public slots:
	void showLoginPanel();
	void showWebView();
	void processLogin(bool success);

private:
	void setProperties();
	void initWidgets();
	void showMessageDialog(QString message);
	void resizeEvent(QResizeEvent *rEvent);

	Ui::CBLoginWidget ui;
	QGraphicsView *view;
	QGraphicsScene *scene;
	CustomProxy *loginpanelProxy;
	CustomProxy *loginwebviewProxy;

	LoginPanel *loginpanel;
	LoginWebView *loginwebview;
	ConfigManager configmanager;

	int loadSplashState;

signals:
	void loginProcessed(QString);

};

#endif
