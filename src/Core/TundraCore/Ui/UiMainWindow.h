// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QMainWindow>
#include <QHash>

#include "UiFwd.h"
#include "TundraCoreApi.h"

class QMenu;
class QAction;
class QIcon;

/// The main application window that is shown when the program starts.
/** This window contains a fullscreen 3D-rendered scene and modules can embed their own Qt widgets in this window. */
class TUNDRACORE_API UiMainWindow : public QMainWindow
{
    Q_OBJECT
    Q_PROPERTY(int desktopWidth READ DesktopWidth)
    Q_PROPERTY(int desktopHeight READ DesktopHeight)

public:
    /// Constructs the main window.
    /** @param owner Framework/owner of the main window. */
    explicit UiMainWindow(Framework *owner);

    /// Loads and applied from config window size, isMaximized, and other settings.
    void LoadWindowSettingsFromFile();

    /// Saves the currently applied window settings to file.
    void SaveWindowSettingsToFile();

    /// Returns the whole desktop width.
    /** @note This function takes into account multiple displays and assumes the displays are side-by-side.
        Perhaps if someone stacked their displays, that assumption would be wrong. */
    static int DesktopWidth();

    /// Returns the whole desktop height, taking into account multiple displays.
    static int DesktopHeight();

public slots:
    /// Ensures that widget gets position withing the desktop (not necessarily within the main window itself).
    /** @param widget Widge to be positioned.
        @param preferredPosition The preferred position for the widget. */
    static void EnsurePositionWithinDesktop(QWidget *widget, QPoint preferredPosition);

    /// Returns if a menu with name is in the main windows menu bar.
    /** @param Name of the menu to search for.
        @return bool If menu with name exists.
        @note name is case sensitive. */
    bool HasMenu(const QString &name) const;

    /// Returns a existing menu, if name does not exist returns null ptr.
    /** @param Name of the menu to return.
        @return QMenu Valid ptr if found, otherwise null ptr.
        @note name is case sensitive. */
    QMenu *GetMenu(const QString &name) const;

    /// Adds a menu with name in the main windows menu bar. If exists returns the existing one.
    /** @param Name of the menu to be added.
        @return QMenu Added or already existing QMenu with the name.
        @note name is case sensitive. */
    QMenu *AddMenu(const QString &name);

    /// Adds a action with name to a sub menu of menuName. Option to provide icon.
    /** @param menuName Name of the sub menu where you want the action to be added.
        @param actionName Name of the action to be added.
        @param optional icon for the action.
        @return QAction the added action.
        @note If menuName does not exist it will be created.
        @note menuName is case sensitive. */
    QAction *AddMenuAction(const QString &menuName, const QString &actionName, const QIcon &icon = QIcon());

signals:
    /// Emitted when the main window is resized.
    /** @param newWidth New width.
        @param netHeight New height. */
    void WindowResizeEvent(int newWidth, int newHeight);

    /// Emitted when the main window is closed.
    void WindowCloseEvent();

private:
    /// Overridden to trigger WindowCloseEvent signal.
    void closeEvent(QCloseEvent *e);

    /// Overridden to trigger WindowResizeEvent signal.
    void resizeEvent(QResizeEvent *e);

    /// Framework ptr.
    Framework *owner;

    /// A hash map to internally track existing menus.
    QHash<QString, QMenu*> menus_;
};
