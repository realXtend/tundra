#ifndef incl_LoadProgressPanel_h
#define incl_LoadProgressPanel_h

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

#endif // incl_LoadProgressPanel_h