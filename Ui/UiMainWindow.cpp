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
#include <QMenu>
#include <QAction>

#include <utility>
#include <iostream>

#include "MemoryLeakCheck.h"

using namespace std;

UiMainWindow::UiMainWindow(Foundation::Framework *owner_) :
    QMainWindow(),
    owner(owner_)
{
    setAcceptDrops(true);
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
    int width = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_width", 800);
    int height = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_height", 600);
    int windowX = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_left", -1);
    int windowY = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_top", -1);
    bool maximized = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_maximized", false);
    bool fullscreen = owner->GetDefaultConfig().DeclareSetting("MainWindow", "fullscreen", false);

    setWindowTitle(owner->ApplicationVersion()->toString());

    width = max(1, min(DesktopWidth(), width));
    height = max(1, min(DesktopHeight(), height));
    windowX = max(0, min(DesktopWidth()-width, windowX));
    windowY = max(25, min(DesktopHeight()-height, windowY));

    resize(width, height);
    move(windowX, windowY);
}

void UiMainWindow::SaveWindowSettingsToFile()
{
    // The values (0, 25) and (-50, -20) below serve to artificially limit the main window positioning into awkward places.
    int windowX = max(0, min(DesktopWidth()-50, pos().x()));
    int windowY = max(25, min(DesktopHeight()-20, pos().y()));
    int width = max(1, min(DesktopWidth()-windowX, size().width()));
    int height = max(1, min(DesktopHeight()-windowY, size().height()));

    // If we are in windowed mode, store the window rectangle for next run.
    if (!isMaximized() && !isFullScreen())
    {
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_width", width);
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_height", height);
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_left", windowX);
        owner->GetDefaultConfig().SetSetting("MainWindow", "window_top", windowY);
    }
    owner->GetDefaultConfig().SetSetting("MainWindow", "window_maximized", isMaximized());
    owner->GetDefaultConfig().SetSetting("MainWindow", "fullscreen", isFullScreen());
}

void UiMainWindow::closeEvent(QCloseEvent *e)
{
    emit WindowCloseEvent();
    e->ignore();
}

void UiMainWindow::resizeEvent(QResizeEvent *e)
{
    emit WindowResizeEvent(width(), height());
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

