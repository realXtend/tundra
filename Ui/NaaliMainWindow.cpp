// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"

#include "NaaliMainWindow.h"
#include "Framework.h"
#include "ConfigurationManager.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QIcon>
#include <QMenuBar>

#include <utility>
#include <iostream>

#include "MemoryLeakCheck.h"

using namespace std;

NaaliMainWindow::NaaliMainWindow(Foundation::Framework *owner_)
:owner(owner_)
{
    setAcceptDrops(true);

	parentWin_ = new QMainWindow();
	parentWin_->setCentralWidget(this);
}

int NaaliMainWindow::DesktopWidth()
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (!desktop)
    {
        cerr << "Error: QApplication::desktop() returned null!";
        return 1024; // Just guess some value for desktop width.
    }
    int width = 0;
    for(int i = 0; i < desktop->screenCount(); ++i)
        width += desktop->screenGeometry(i).width();

    return width;
}

int NaaliMainWindow::DesktopHeight()
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (!desktop)
    {
        cerr << "Error: QApplication::desktop() returned null!";
        return 768; // Just guess some value for desktop height.
    }
    return desktop->screenGeometry().height();
}

void NaaliMainWindow::LoadWindowSettingsFromFile()
{
    setWindowTitle("Tundra v1.0.3");
	
	QPoint pos;
	int win_width;
	int win_height;

	//QSettings
	if (owner->IsEditionless())
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiPlayerSettings");
		pos = settings.value("win_pos", QPoint(200, 200)).toPoint();
		win_width = settings.value("win_width", 800).toInt();
		win_height = settings.value("win_height", 600).toInt();
	}
	else
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiSettings");
		pos = settings.value("win_pos", QPoint(200, 200)).toPoint();
		win_width = settings.value("win_width", 800).toInt();
		win_height = settings.value("win_height", 600).toInt();
	}

		// Create window title
        std::string group = Foundation::Framework::ConfigurationGroup();
		std::string version_major = owner->GetDefaultConfig().GetSetting<std::string>(group, "version_major");
        std::string version_minor = owner->GetDefaultConfig().GetSetting<std::string>(group, "version_minor");
        std::string window_titleaux = owner->GetDefaultConfig().GetSetting<std::string>(group, "window_title") + " " + version_major + "." + version_minor;
		QString window_title = QString::fromStdString(window_titleaux);
		

		//Assign parameters to our window
		parentWin_->setWindowTitle(window_title);
		//parentWin_->setMinimumSize(width,height);		
		parentWin_->setDockNestingEnabled(true);

		//Set size
		parentWin_->resize(win_width,win_height);
		parentWin_->move(pos);
		
		//Menu bar for Qwin this Mac support
		 QMenuBar *menuBar = new QMenuBar(parentWin_);
		 menuBar->heightForWidth(500);
		 parentWin_->setMenuBar(menuBar);
}

void NaaliMainWindow::SaveWindowSettingsToFile()
{
	//Not needed any more since QMainWindow restoreState is used in uimodule.
}

void NaaliMainWindow::closeEvent(QCloseEvent *e)
{
    emit WindowCloseEvent();
    e->ignore();
}

void NaaliMainWindow::resizeEvent(QResizeEvent *e)
{
    emit WindowResizeEvent(width(), height());
}

//$ BEGIN_MOD $
void NaaliMainWindow::ToggleFullScreen()
{
    if (parentWin_->isFullScreen())
        parentWin_->showNormal();
    else
        parentWin_->showFullScreen();
}
//$ END_MOD $

