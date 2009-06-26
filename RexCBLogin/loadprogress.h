#ifndef LOGINPROGRESS_H
#define LOGINPROGRESS_H

#include "ui_loadprogress.h"
#include <QDialog>
#include <QDesktopWidget>

class LoadProgressPanel
	: public QDialog
{
	Q_OBJECT

public:
	LoadProgressPanel(QWidget *parent = 0);
	Ui_LoadProgress *ui;

private:
	QRect screen;
};

#endif