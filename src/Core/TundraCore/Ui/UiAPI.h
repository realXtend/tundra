// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QObject>
#include <QPointer>
#include <QWidget>

#include "UiFwd.h"
#include "TundraCoreApi.h"

class QMenu;

/// Used as a viewport for the main QGraphicsView.
/** Its purpose is to disable all automatic drawing of the QGraphicsView to screen so that
 we can composite an Ogre 3D render with the Qt widgets added to a QGraphicsScene. */
class TUNDRACORE_API SuppressedPaintWidget : public QWidget
{
public:
    SuppressedPaintWidget(QWidget *parent = 0, Qt::WindowFlags f = 0);
    virtual ~SuppressedPaintWidget() {}
    
protected:
    virtual bool event(QEvent *event);
    virtual void paintEvent(QPaintEvent *event);
};

/// Provides functions for managing the currently opened windows in the application main window.
/** @todo More detailed description.
    @todo Make a new .dox file for UiAPI. */
class TUNDRACORE_API UiAPI : public QObject
{
    Q_OBJECT

public:
    /// Constructs the UI API.
    /** param owner Owner framework. */
    explicit UiAPI(Framework *owner);

    /// Destroys main window and viewport.
    ~UiAPI();

    void Reset();

public slots:
    /// Returns the widget that is the main window.
    /** @return Main window pointer, or null pointer if we're running in headless mode. */
    UiMainWindow *MainWindow() const;

    /// This widget shows all the user-added UI that has been added to the screen. It is contained as a child of UiMainWindow.
    /** @return Graphics view pointer, or null pointer if we're running in headless mode. */
    UiGraphicsView *GraphicsView() const;

    /// Returns the main QGraphicsScene, which hosts all the user-added widgets.
    /** @return Graphics scene pointer, or null pointer if we're running in headless mode. */
    QGraphicsScene *GraphicsScene() const;

    /// Adds (embeds) widget to the main graphics scene.
    /** Creates a proxy widget for the @c widget and adds it to the main graphics scene. The graphics scene
        (which can be retrieved by using GraphicsScene()) takes ownership of the proxy widget and handles
        its deletion when the scene itself is deleted. If you delete the original widget, the corresponding
        proxy widget is deleted automatically.

        The created proxy widget maintains symmetry with the original widget for the following states:
        state, enabled, visible, geometry, layoutDirection, style, palette, font, cursor, sizeHint,
        getContentsMargins and windowTitle

        If you want to add full screen widget that is resized automatically to fit the screen
        when scene rectangle changes, remember to set Qt::FullScreen window state for the widget.

        @param  widget Widget to be embedded.
        @param  flags Window flags. Qt::Dialog is used as default.
                It creates movable proxy widget which has title bar and frames.
                If you want add widget without title bar and frames, use Qt::Widget.
                For further information, see http://doc.qt.nokia.com/4.7/qt.html#WindowType-enum
        @return Proxy widget of the added widget.

        @bug flags is always the default value when calling this function from sctips! */
    UiProxyWidget *AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    /// Adds user-created proxy widget to the scene.
    /** @param widget Proxy widget.
        @return True if the addition was successful, false otherwise. */
    bool AddProxyWidgetToScene(UiProxyWidget *proxy);

    /// Removes widget's proxy widget from the scene.
    /** @param widget Widget. */
    void RemoveWidgetFromScene(QWidget *widget);
    /// @overload
    /** Removes proxy widget from the scene.
        @param widget Proxy widget. */
    void RemoveWidgetFromScene(QGraphicsProxyWidget *widget);

    /// Removes proxy widget from scene.
    /** @param widget Proxy widget.
        @todo Make function overloads work with QtScript and remove this function. */
    void RemoveProxyWidgetFromScene(QGraphicsProxyWidget *widget) { return RemoveWidgetFromScene(widget); }

    /// Shows the widget's proxy widget in the scene.
    /** @param widget Widget. */
    void ShowWidget(QWidget *widget) const;

    /// Hides the widget's proxy widget in the scene.
    /** @param widget Widget. */
    void HideWidget(QWidget *widget) const;

    /// Brings the widget's proxy widget to front in the graphics scene and sets focus to it.
    /** @param widget Widget. */
    void BringWidgetToFront(QWidget *widget) const;
    /// @overload
    /** Brings the widget's proxy widget to front in the graphics scene and sets focus to it.
        @param widget Proxy widget. */
    void BringWidgetToFront(QGraphicsProxyWidget *widget) const { return BringProxyWidgetToFront(widget); }

    /// Brings the proxy widget to front in the graphics scene and sets focus to it.
    /** @param widget Proxy widget.
        @todo Make function overloads work with QtScript and remove this function. */
    void BringProxyWidgetToFront(QGraphicsProxyWidget *widget) const;

    /// Loads widget from .ui file and adds it to the graphics scene.
    /** @note This is a convenience function for scripting languages.
        @note Favor usage of QtUiAsset::Instantiate in scripts.
        @param filePath .ui file location.
        @param parent Parent widget.
        @param addToScene Do we want to add new widget to the graphics scene.
        @return loaded widget's pointer (null if loading failed). */
    QWidget *LoadFromFile(const QString &filePath, bool addToScene = true, QWidget *parent = 0);

    /// Emits a signal that can be used to attach context menu actions for specific object types.
    /** @param menu Menu to which possible functionality can be appended.
        @param targets List of target objects for which the context menu is about to open. */
    void EmitContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets);

signals:
    /// Signals that context menu @c menu is about to open for specific objects.
    /** @param menu Menu to which append functionalities.
        @param targets List of target objects for which the context menu is about to open. */
    void ContextMenuAboutToOpen(QMenu *menu, QList<QObject *> targets);

private slots:
    /// Removes proxy widget from internally maintained lists upon destruction.
    void OnProxyDestroyed(QObject *obj);

    /// Performs different operations for proxy widgets when scene rectangle is changed.
    /** F.ex. resizes full screen widgets to fit the screen.
        @param rect New scene rectangle. */
    void OnSceneRectChanged(const QRectF &rect);

    /// Deletes widget and the corresponding proxy widget if widget has WA_DeleteOnClose on.
    /** The caller of this slot is retrieved by using QObject::sender(). */
    void DeleteCallingWidgetOnClose();

private:
    Framework *owner; ///< Owner framework.
    QPointer<UiMainWindow> mainWindow; ///< The main window.
    UiGraphicsView *graphicsView; ///< Widget which shows contents of the graphics scene.
    QGraphicsScene *graphicsScene; ///< The graphics scene, host all the user-added widgets.
    QPointer<QWidget> viewportWidget; ///< Viewport for the graphics scene.
    QList<QGraphicsProxyWidget *> widgets; ///< Internal list of proxy widgets in scene.
    QList<QGraphicsProxyWidget *> fullScreenWidgets; ///< List of full screen widgets.
};
