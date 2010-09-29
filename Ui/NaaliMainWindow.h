#ifndef incl_Core_NaaliMainWindow_h
#define incl_Core_NaaliMainWindow_h

#include <QMainWindow>

#include "NaaliUiFwd.h"
#include "UiApi.h"

/** NaaliMainWindow is the main application window that is shown when the program starts. This window contains a fullscreen
3D-rendered scene and modules can embed their own Qt widgets in this window. */
class UI_API NaaliMainWindow : public QWidget /*QMainWindow*/
{
    Q_OBJECT;
public:
    explicit NaaliMainWindow(Foundation::Framework *owner_);

    void LoadWindowSettingsFromFile();
    void SaveWindowSettingsToFile();

    static int DesktopWidth();
    static int DesktopHeight();

signals:
    void WindowResizeEvent(int newWidth, int newHeight);
    void WindowCloseEvent();

private:
    void closeEvent(QCloseEvent *e);
    void resizeEvent(QResizeEvent *e);

    Foundation::Framework *owner;
};

#endif
