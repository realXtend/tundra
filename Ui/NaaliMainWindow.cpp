// For conditions of distribution and use, see copyright notice in license.txt

#include "DebugOperatorNew.h"

#include "NaaliMainWindow.h"
#include "Framework.h"
#include "ConfigAPI.h"

#include <QCloseEvent>
#include <QDesktopWidget>
#include <QApplication>
#include <QIcon>

#include <utility>
#include <iostream>

#include "MemoryLeakCheck.h"

using namespace std;

NaaliMainWindow::NaaliMainWindow(Foundation::Framework *owner_)
:owner(owner_)
{
    setAcceptDrops(true);
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
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, ConfigAPI::SECTION_UI);
    int width = owner->Config()->Get(configData, "window width", 800).toInt();
    int height = owner->Config()->Get(configData, "window height", 600).toInt();
    int windowX = owner->Config()->Get(configData, "window left", -1).toInt();
    int windowY = owner->Config()->Get(configData, "window top", -1).toInt();
    bool maximized = owner->Config()->Get(configData, "window maximized", false).toBool();
    bool fullscreen = owner->Config()->Get(configData, "fullscreen", false).toBool();

    setWindowTitle(owner->Config()->GetApplicationIdentifier());

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

void NaaliMainWindow::closeEvent(QCloseEvent *e)
{
    emit WindowCloseEvent();
    e->ignore();
}

void NaaliMainWindow::resizeEvent(QResizeEvent *e)
{
    emit WindowResizeEvent(width(), height());
}

