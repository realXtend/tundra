// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_NaaliMainWindow_h
#define incl_Core_NaaliMainWindow_h

#include <QMainWindow>

#include "NaaliUiFwd.h"
#include "UiApi.h"

/// NaaliMainWindow is the main application window that is shown when the program starts.
/** This window contains a fullscreen 3D-rendered scene and modules can embed their own Qt widgets in this window.
*/
class UI_API NaaliMainWindow : public QWidget /*QMainWindow*/
{
    Q_OBJECT;
public:
    /// Constructs the main window.
    /** @param owner_ Framework/owner of the main window.
    */
    explicit NaaliMainWindow(Foundation::Framework *owner_);

    /// Loads window settings from the config file.
    void LoadWindowSettingsFromFile();

    /// Saves current windows settings to the config file.
    void SaveWindowSettingsToFile();

    /// Returns OS desktop width.
    static int DesktopWidth();

    /// Returns OS desktop height.
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
    void closeEvent(QCloseEvent *e);
    void resizeEvent(QResizeEvent *e);

    Foundation::Framework *owner;
};

#endif
