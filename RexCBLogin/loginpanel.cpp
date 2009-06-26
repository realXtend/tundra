#include "loginpanel.h"

#include <QtGui>

LoginPanel::LoginPanel(QWidget *parent)
	: QWidget(parent)
{
	ui = new Ui_LoginPanel;
	ui->setupUi(this);

	setProperties();
	connectSignals();
}

void LoginPanel::setProperties() 
{
	setObjectName("logindata");
	ui->comboBoxWorlds->addItem("");
	ui->comboBoxOpenIDs->addItem("");
}

void LoginPanel::connectSignals()
{
	QObject::connect(ui->comboBoxWorlds, SIGNAL( currentIndexChanged(QString) ), SLOT( setWorldAddress(QString) ));
	QObject::connect(ui->comboBoxOpenIDs, SIGNAL( currentIndexChanged(QString) ), SLOT( setOpenIDAddress(QString) ));
}

void LoginPanel::setComboBoxItems(QString boxName, QStringList *items)
{
	if (boxName == "worlds")
		ui->comboBoxWorlds->addItems(*items);
	else if (boxName == "openids")
		ui->comboBoxOpenIDs->addItems(*items);
}


////////////////////////////////////////////
//             PUBLIC SLOTS               //
////////////////////////////////////////////

QString LoginPanel::getWorldUri() 
{
	return ui->lineEditWorld->text();
}

QString LoginPanel::getOpenIDUri()
{
	return ui->lineEditOpenID->text();
}

void LoginPanel::setWorldAddress(QString uri)
{
	ui->lineEditWorld->setText(uri);
}

void LoginPanel::setOpenIDAddress(QString uri)
{
	ui->lineEditOpenID->setText(uri);
}