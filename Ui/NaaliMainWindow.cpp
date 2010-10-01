#include "NaaliMainWindow.h"
#include "Framework.h"
#include "ConfigurationManager.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QApplication>

#include <utility>
#include <iostream>

using namespace std;

NaaliMainWindow::NaaliMainWindow(Foundation::Framework *owner_)
:owner(owner_)
{
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
    int width = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_width", 800);
    int height = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_height", 600);
    int windowX = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_left", -1);
    int windowY = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_top", -1);
    bool maximized = owner->GetDefaultConfig().DeclareSetting("MainWindow", "window_maximized", false); 
    bool fullscreen = owner->GetDefaultConfig().DeclareSetting("MainWindow", "fullscreen", false);

    width = max(1, min(DesktopWidth(), width));
    height = max(1, min(DesktopHeight(), height));
    windowX = max(0, min(DesktopWidth()-width, windowX));
    windowY = max(25, min(DesktopHeight()-height, windowY));

    resize(width, height);
    move(windowX, windowY);
}

void NaaliMainWindow::SaveWindowSettingsToFile()
{
    // The values (0, 25) and (-50, -20) below serve to artificially limit the main window positioning into awkward places.
    int windowX = max(0, min(DesktopWidth()-50, geometry().x()));
    int windowY = max(25, min(DesktopHeight()-20, geometry().y()));
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

void NaaliMainWindow::closeEvent(QCloseEvent *e)
{
    emit WindowCloseEvent();
    e->ignore();
}

void NaaliMainWindow::resizeEvent(QResizeEvent *e)
{
    emit WindowResizeEvent(width(), height());
}