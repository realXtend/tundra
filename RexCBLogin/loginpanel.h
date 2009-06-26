#ifndef LOGINPANEL_H
#define LOGINPANEL_H

#include "ui_loginpanel.h"
#include <QWidget>

class LoginPanel 
	: public QWidget
{
	Q_OBJECT

public:
	LoginPanel(QWidget *parent = 0);
	Ui_LoginPanel *ui;
	void setComboBoxItems(QString boxName, QStringList *items);

public slots:
	void setProperties();
	void connectSignals();
	QString getWorldUri();
	QString getOpenIDUri();
	void setWorldAddress(QString uri);
	void setOpenIDAddress(QString uri);

};

#endif