// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"

#include "UiMainWindow.h"
#include "Framework.h"
#include "VersionInfo.h"
#include "ConfigurationManager.h"
#include <QCloseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QMenuBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

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


UiMainWindow::UiMainWindow(Foundation::Framework *owner_) :
    QMainWindow(),
    owner(owner_)
{
    setAcceptDrops(true);
    setCentralWidget(new NewCentralWid(this));
}

int UiMainWindow::DesktopWidth()
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

int UiMainWindow::DesktopHeight()
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (!desktop)
    {
        cerr << "Error: QApplication::desktop() returned null!";
        return 768; // Just guess some value for desktop height.
    }
    return desktop->screenGeometry().height();
}

void UiMainWindow::LoadWindowSettingsFromFile()
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
    setWindowTitle(owner->ApplicationVersion()->toString());

    windowY = max(25, min(DesktopHeight()-height, windowY));
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

void UiMainWindow::SaveWindowSettingsToFile()
{
	//Not needed any more since QMainWindow restoreState is used in uimodule.
}

void UiMainWindow::closeEvent(QCloseEvent *e)
{
    emit WindowCloseEvent();
    e->ignore();
}

void UiMainWindow::resizeEvent(QResizeEvent *e)
{
    emit WindowResizeEvent(centralWidget()->width(), centralWidget()->height());
}

bool UiMainWindow::HasMenu(const QString &name)
{
    if (!menuBar())
        return false;

    QList<QString> menuNames = menus_.keys();
    return menuNames.contains(name);
}

QMenu *UiMainWindow::GetMenu(const QString &name)
{
    if (!menuBar())
        return 0;

    if (HasMenu(name))
        return menus_[name];
    return 0;
}

QMenu *UiMainWindow::AddMenu(const QString &name)
{
    if (!menuBar())
        return 0;

    if (HasMenu(name))
        return menus_[name];
    QMenu *menu = menuBar()->addMenu(name);
    menus_[name] = menu;
    return menu;
}

QAction *UiMainWindow::AddMenuAction(const QString &menuName, const QString &actionName, const QIcon &icon)
{
    if (!menuBar())
        return 0;

    QMenu *menu = 0;
    if (HasMenu(menuName))
        menu = menus_[menuName];
    else
        menu = AddMenu(menuName);

    if (!menu)
        return 0;
    return menu->addAction(icon, actionName);
}

void NaaliMainWindow::SizeOfCentralWidgetChanged(){
    emit WindowResizeEvent(centralWidget()->width(), centralWidget()->height());

