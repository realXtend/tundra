#include "../Core/DebugOperatorNew.h"
#include "loginpanel.h"

#include <QtGui>
#include "../Core/MemoryLeakCheck.h"

LoginPanel::LoginPanel(QWidget *parent)
	: QWidget(parent), ui(0)
{
	ui = new Ui_LoginPanel();
	ui->setupUi(this);

	setProperties();
	connectSignals();
}

LoginPanel::~LoginPanel()
{
    delete ui;
}

void LoginPanel::setProperties() 
{
	setObjectName("logindata");
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