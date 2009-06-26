#include "loadprogress.h"

LoadProgressPanel::LoadProgressPanel(QWidget *parent)
	: QDialog(parent)
{
	ui = new Ui_LoadProgress;
	ui->setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
}