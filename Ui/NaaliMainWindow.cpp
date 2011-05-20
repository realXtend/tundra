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

class NewCentralWid : public QWidget 
{
public:
    NewCentralWid(NaaliMainWindow *parent = 0, Qt::WindowFlags f = 0);
    virtual ~NewCentralWid() {}

protected:
    /// Overridden to trigger WindowResizeEvent signal.
    void resizeEvent(QResizeEvent *e);
    NaaliMainWindow *parent_;

};

NewCentralWid::NewCentralWid(NaaliMainWindow *parent, Qt::WindowFlags f)
: QWidget(parent, f),
parent_(parent)
{
}

void NewCentralWid::resizeEvent(QResizeEvent *e)
{
    parent_->SizeOfCentralWidgetChanged();
}


NaaliMainWindow::NaaliMainWindow(Foundation::Framework *owner_)
:owner(owner_)
{
    setAcceptDrops(true);
    setCentralWidget(new NewCentralWid(this));
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
	QPoint pos;
	int win_width;
	int win_height;
	bool window_fullscreen = false;

	//QSettings
	if (owner->IsEditionless())
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiPlayerSettings");
		pos = settings.value("win_pos", QPoint(200, 200)).toPoint();
		win_width = settings.value("win_width", 1024).toInt();
		win_height = settings.value("win_height", 768).toInt();
		window_fullscreen = settings.value("win_fullscreen", false).toBool();
	}
	else
	{
		QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, "configuration/UiSettings");
		pos = settings.value("win_pos", QPoint(200, 200)).toPoint();
		win_width = settings.value("win_width", 1024).toInt();
		win_height = settings.value("win_height", 768).toInt();
		window_fullscreen = settings.value("win_fullscreen", false).toBool();
	}

		// Create window title
        /*std::string group = Foundation::Framework::ConfigurationGroup();
		std::string version_major = owner->GetDefaultConfig().GetSetting<std::string>(group, "version_major");
        std::string version_minor = owner->GetDefaultConfig().GetSetting<std::string>(group, "version_minor");
        std::string window_titleaux = owner->GetDefaultConfig().GetSetting<std::string>(group, "window_title") + " " + version_major + "." + version_minor;
		QString window_title = QString::fromStdString(window_titleaux);*/
		

		//Assign parameters to our window
		setWindowTitle("Tundra 1.0.3");
		setDockNestingEnabled(true);

		//Menu bar for Qwin this Mac support
		QMenuBar *menuBar = new QMenuBar(this);
		menuBar->heightForWidth(500);
		setMenuBar(menuBar);

		resize(win_width,win_height);

		if (window_fullscreen)
			showFullScreen();
		else
			move(pos);
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
    emit WindowResizeEvent(centralWidget()->width(), centralWidget()->height());
}

void NaaliMainWindow::SizeOfCentralWidgetChanged(){
    emit WindowResizeEvent(centralWidget()->width(), centralWidget()->height());
}

