// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_NaaliMainWindow_h
#define incl_Core_NaaliMainWindow_h

#include <QMainWindow>

#include "NaaliUiFwd.h"
#include "UiApi.h"

/** NaaliMainWindow is the main application window that is shown when the program starts. This window contains a fullscreen
3D-rendered scene and modules can embed their own Qt widgets in this window.
\todo In the future, this class should derive from QMainWindow to be able to use its functionality. */
class UI_API NaaliMainWindow : public QWidget /*QMainWindow*/
{
    Q_OBJECT;
public:
    /// Constructs the main window.
    /** @param owner_ Framework/owner of the main window.
    */
    explicit NaaliMainWindow(Foundation::Framework *owner_);

    /// Loads and applied from Naali Config window size, isMaximized, and other settings.
    void LoadWindowSettingsFromFile();
    /// Saves the currently applied window settings to file.
    void SaveWindowSettingsToFile();

    /// Returns the whole desktop width. \note This function takes into account multiple displays and assumes the displays are
    /// side-by-side. Perhaps if someone stacked their displays, that assumption would be wrong.
    static int DesktopWidth();
    /// Returns the whole desktop height, taking into account multiple displays.
    static int DesktopHeight();

signals:
    /// Emitted when the main window is resized.
    /** @param newWidth New width.
        @param netHeight New height.
    */
    void WindowResizeEvent(int newWidth, int newHeight);

    /// Emitted when the main window is closed.
    void WindowCloseEvent();

private:
    /// Overridden to trigger WindowCloseEvent signal.
    void closeEvent(QCloseEvent *e);

    /// Overridden to trigger WindowResizeEvent signal.
    void resizeEvent(QResizeEvent *e);

    Foundation::Framework *owner;
};

#endif
