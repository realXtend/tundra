// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiMainWindow.h"
#include "Framework.h"
#include "ConfigAPI.h"
#include "LoggingFunctions.h"
#include "Application.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QMenuBar>

#include <utility>
#include <iostream>

#include "MemoryLeakCheck.h"

using namespace std;

UiMainWindow::UiMainWindow(Framework *owner_)
:owner(owner_)
{
    setAcceptDrops(true);
}

int UiMainWindow::DesktopWidth()
{
    QDesktopWidget *desktop = QApplication::desktop();
    if (!desktop)
    {
        std::string error("UiMainWindow::DesktopWidth: QApplication::desktop() returned null!");
        LogError(error);
        cerr << error << endl;
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
        std::string error("UiMainWindow::DesktopHeight: QApplication::desktop() returned null!");
        LogError(error);
        cerr << error << endl;
        return 768; // Just guess some value for desktop height.
    }
    return desktop->screenGeometry().height();
}

void UiMainWindow::LoadWindowSettingsFromFile()
{
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_UI);
    int width = owner->Config()->Get(configData, "window width", 800).toInt();
    int height = owner->Config()->Get(configData, "window height", 600).toInt();
    int windowX = owner->Config()->Get(configData, "window left", -1).toInt();
    int windowY = owner->Config()->Get(configData, "window top", -1).toInt();
    bool maximized = owner->Config()->Get(configData, "window maximized", false).toBool();
    bool fullscreen = owner->Config()->Get(configData, "fullscreen", false).toBool();

#ifdef Q_WS_MAC
    // Fullscreen causes crash on Mac OS X. See https://github.com/realXtend/naali/issues/522
    fullscreen = false;
#endif

    // Round up width to next 16 to avoid startup artifacts with the Ogre blit
    width += 15;
    width &= 0xfffffff0;
    
    setWindowTitle(Application::FullIdentifier());
    
    width = max(1, min(DesktopWidth(), width));
    height = max(1, min(DesktopHeight(), height));
    windowX = max(0, min(DesktopWidth()-width, windowX));
    windowY = max(25, min(DesktopHeight()-height, windowY));

    resize(width, height);
    move(windowX, windowY);

    if (fullscreen)
        showFullScreen();
    else if (maximized)
        showMaximized();
    else
        showNormal();
}

void UiMainWindow::SaveWindowSettingsToFile()
{
    // The values (0, 25) and (-50, -20) below serve to artificially limit the main window positioning into awkward places.
    int windowX = max(0, min(DesktopWidth()-50, pos().x()));
    int windowY = max(25, min(DesktopHeight()-20, pos().y()));
    int width = max(1, min(DesktopWidth()-windowX, size().width()));
    int height = max(1, min(DesktopHeight()-windowY, size().height()));

    // If we are in windowed mode, store the window rectangle for next run.
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_UI);
    if (!isMaximized() && !isFullScreen())
    {
        owner->Config()->Set(configData, "window width", width);
        owner->Config()->Set(configData, "window height", height);
        owner->Config()->Set(configData, "window left", windowX);
        owner->Config()->Set(configData, "window top", windowY);
    }
    owner->Config()->Set(configData, "window maximized", isMaximized());
    owner->Config()->Set(configData, "fullscreen", isFullScreen());
}

void UiMainWindow::closeEvent(QCloseEvent *e)
{
    emit WindowCloseEvent();
    e->ignore();
}

void UiMainWindow::resizeEvent(QResizeEvent * /*e*/)
{
    emit WindowResizeEvent(width(), height());
}

void UiMainWindow::EnsurePositionWithinDesktop(QWidget *widget, QPoint pos)
{
    if (!widget)
    {
        LogError("UiMainWindow::EnsurePositionWithinDesktop: null widget passed!");
        return;
    }
    int xMax = DesktopWidth(), yMax = DesktopHeight();

    // Allow setting windows partially outside the screen.
    // Apply an arbitrary guardband of (50x100) along the screen borders.
    const int w = max(widget->width()/2, 50);
    const int h = max(widget->height()/2, 100);
    if (pos.x() + w > xMax)
        pos.setX(xMax - w);
    if (pos.y() + h > yMax)
        pos.setY(yMax - h);
    if (pos.x() < 0)
        pos.setX(0);
    if (pos.y() < 0)
        pos.setY(0);
    widget->move(pos);
}

bool UiMainWindow::HasMenu(const QString &name) const
{
    if (!menuBar())
        return false;

    QList<QString> menuNames = menus_.keys();
    return menuNames.contains(name);
}

QMenu *UiMainWindow::GetMenu(const QString &name) const
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
