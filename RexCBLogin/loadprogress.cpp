#include "../Core/DebugOperatorNew.h"
#include "loadprogress.h"
#include "../Core/MemoryLeakCheck.h"

LoadProgressPanel::LoadProgressPanel(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui_LoadProgress();
	ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
}

LoadProgressPanel::~LoadProgressPanel()
{
    delete ui;
}