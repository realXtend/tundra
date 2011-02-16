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
    /*int width = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_width", 800);
    int height = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_height", 600);
    int windowX = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_left", -1);
    int windowY = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_top", -1);
    bool maximized = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_maximized", false);
    bool fullscreen = owner->GetDefaultConfig().DeclareSetting("MainWindow", "fullscreen", false);
/* Don't use the version from the configuration, since the user may have several Naali versions installed simultaneously.
    std::string title = owner->GetDefaultConfig().GetSetting<std::string>("Foundation", "window_title");
    std::string version_major = owner->GetDefaultConfig().GetSetting<std::string>("Foundation", "version_major");
    std::string version_minor = owner->GetDefaultConfig().GetSetting<std::string>("Foundation", "version_minor");

    setWindowTitle(QString("%1 %2.%3").arg(title.c_str(), version_major.c_str(), version_minor.c_str()));
*/
    setWindowTitle("Tundra v1.0.0");

    /*width = max(1, min(DesktopWidth(), width));
    height = max(1, min(DesktopHeight(), height));
    windowX = max(0, min(DesktopWidth()-width, windowX));
    windowY = max(25, min(DesktopHeight()-height, windowY));

    resize(width, height);
    move(windowX, windowY);*/

	//QSettings
		QSettings settings("Naali UIExternal2", "UiExternal Settings");
		QPoint pos = settings.value("win_pos", QPoint(200, 200)).toPoint();
		QSize size = settings.value("win_size", QSize(400, 400)).toSize();		
		

		// Create window title
        std::string group = Foundation::Framework::ConfigurationGroup();
		std::string version_major = owner->GetDefaultConfig().GetSetting<std::string>(group, "version_major");
        std::string version_minor = owner->GetDefaultConfig().GetSetting<std::string>(group, "version_minor");
        std::string window_titleaux = owner->GetDefaultConfig().GetSetting<std::string>(group, "window_title") + " " + version_major + "." + version_minor;
		QString window_title = QString::fromStdString(window_titleaux);
		

		//Get config parameters
		int width = owner->GetDefaultConfig().DeclareSetting("UiQMainWindow", "window_width", 800);
        int height = owner->GetDefaultConfig().DeclareSetting("UiQMainWindow", "window_height", 600);
	
#ifdef PLAYER_VIEWER
        bool maximized = owner->GetDefaultConfig().DeclareSetting("UiQMainWindow", "window_maximized", true);
		bool fullscreen = owner->GetDefaultConfig().DeclareSetting("UiQMainWindow", "fullscreen", true);
#else
		bool maximized = owner->GetDefaultConfig().DeclareSetting("UiQMainWindow", "window_maximized", false);
        bool fullscreen = owner->GetDefaultConfig().DeclareSetting("UiQMainWindow", "fullscreen", false);
#endif

		//Assign parameters to our window
		parentWin_->setWindowTitle(window_title);
		parentWin_->setMinimumSize(width,height);
		parentWin_->setDockNestingEnabled(true);
		//setCentralWidget(owner->GetNaaliApplication());//GetFramework()->GetMainWindow());

		//Set size
		parentWin_->resize(size);
		parentWin_->move(pos);
		
		//Menu bar for Qwin this Mac support
		 QMenuBar *menuBar = new QMenuBar(parentWin_);
		 menuBar->heightForWidth(500);
		 parentWin_->setMenuBar(menuBar);
		
		 //Here?
		 parentWin_->restoreState(settings.value("win_state", QByteArray()).toByteArray());
}

void NaaliMainWindow::SaveWindowSettingsToFile()
{
    // The values (0, 25) and (-50, -20) below serve to artificially limit the main window positioning into awkward places.
    int windowX = max(0, min(DesktopWidth()-50, parentWin_->pos().x()));
    int windowY = max(25, min(DesktopHeight()-20, parentWin_->pos().y()));
    int width = max(1, min(DesktopWidth()-windowX, parentWin_->size().width()));
    int height = max(1, min(DesktopHeight()-windowY, parentWin_->size().height()));

    // If we are in windowed mode, store the window rectangle for next run.
    if (!isMaximized() && !isFullScreen())
    {
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_width", width);
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_height", height);
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_left", windowX);
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_top", windowY);
    }
    owner->GetDefaultConfig().SetSetting("MainWindow", "window_maximized", parentWin_->isMaximized());
    owner->GetDefaultConfig().SetSetting("MainWindow", "fullscreen", parentWin_->isFullScreen());

	//Save settings with QtSettings..
	QSettings settings("Naali UIExternal2", "UiExternal Settings");
	settings.setValue("win_pos", parentWin_->pos());
	settings.setValue("win_size", parentWin_->size());
	//settings.setValue("win_state", saveState());
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

