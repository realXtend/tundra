// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Ui_UiAPI_h
#define incl_Ui_UiAPI_h

#include <QObject>
#include <QPointer>
#include <memory>

#include "UiFwd.h"
#include "UiApiExport.h"

/// UiAPI is the core API object exposed for all UI-related functionality.
class UI_API UiAPI : public QObject
{
    Q_OBJECT;

public:
    explicit UiAPI(Foundation::Framework *owner);
    ~UiAPI();

public slots:
    /// Returns the widget that is the main window of Naali.
    NaaliMainWindow *MainWindow() const;

    /// This widget shows all the user-added UI that has been added to the screen. It is contained as a child of NaaliMainWindow.
    NaaliGraphicsView *GraphicsView() const;

    /// Returns the Naali main QGraphicsScene, which hosts all the user-added widgets.
    QGraphicsScene *GraphicsScene() const;

    /// Adds widget to the graphics scene.
    /** The created graphics proxy widget maintains symmetry with the original widget for the following states:
        state, enabled, visible, geometry, layoutDirection, style, palette, font, cursor, sizeHint,
        getContentsMargins and windowTitle

        If you want to add full screen widget that is resized automatically to fit the screen
        when scene rectangle changes, remember to set Qt::FullScreen window state for the widget.

        @param  widget Widget.
        @param  flags Window flags. Qt::Dialog is used as default.
                It creates movable proxy widget which has title bar and frames.
                If you want add widget without title bar and frames, use Qt::Widget.
                For further information, see http://doc.qt.nokia.com/4.6/qt.html#WindowType-enum
        @return Proxy widget of the added widget.
    */
//    UiProxyWidget *AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    /// For QtScript compability.
//    UiProxyWidget *AddWidgetToSceneRaw(QWidget *widget, int flags = (int)Qt::Dialog) { return AddWidgetToScene(widget, (Qt::WindowFlags)flags); }

    /// Adds user-created UiProxyWidget to the scene.
    /** @param widget Proxy widget.
    */
//    bool AddWidgetToScene(UiProxyWidget *proxy);

//    bool AddProxyWidgetToScene(UiProxyWidget *proxy) { return AddWidgetToScene(proxy); }

    /// Removes widget from the scene.
    /** @param widget Widget.
    */
//    void RemoveWidgetFromScene(QWidget *widget);

    /// This is an overloaded function.
    /** @param widget Proxy widget.
    */
//    void RemoveWidgetFromScene(QGraphicsProxyWidget *widget);

    /// Shows the widget's proxy widget in the scene.
    /** @param widget Widget.
    */
//    void ShowWidget(QWidget *widget) const;

    /// Hides the widget's proxy widget in the scene.
    /** @param widget Widget.
    */
//    void HideWidget(QWidget *widget) const;

    /// Brings the widget's proxy widget to front in the and sets focus to it.
    /** @param widget Widget.
    */
//    void BringWidgetToFront(QWidget *widget) const;

    /// This is an overloaded function.
    /** Brings the proxy widget to front in the scene and sets focus to it.
        @param widget Proxy widget.
    */
//    void BringWidgetToFront(QGraphicsProxyWidget *widget) const;

    /// Loads widget from .ui file and adds it to the graphics scene.
    /** @note This is a convenience function for scripting languages.
        @param file_path ui file location.
        @param parent Parent widget.
        @param add_to_scene Do we want to add new widget to the graphics scene.
        @return loaded widget's pointer (null if loading failed).
    */
//    QWidget *LoadFromFile(const QString &file_path,  bool add_to_scene = true, QWidget *parent = 0);

//private slots:
    /// Remove proxywidget from internally maintained lists upon destruction.
//    void OnProxyDestroyed(QObject *obj);

    /// Performs different operations for proxy widgets when scene rectangle is changed, f.ex. resizes
    /** full screen widgets to fit the screen.
        @param rect New scene rectangle.
    */
//    void SceneRectChanged(const QRectF &rect);

    /// Deletes widget and the corresponding proxy widget if widget has WA_DeleteOnClose on.
    /** The caller of this slot is retrieved by using QObject::sender().
    */
//    void DeleteCallingWidgetOnClose();

private:
    Foundation::Framework *owner; ///< Owner framework.
    QPointer<NaaliMainWindow> mainWindow; ///< The main window.
    NaaliGraphicsView *graphicsView; ///< Widget which shows contents of the graphics scene.
    QGraphicsScene *graphicsScene; ///< The graphics scene, host all the user-added widgets.
    QPointer<QWidget> viewportWidget; ///< Viewport for the graphics scene.
//    QList<QGraphicsProxyWidget *> widgets_; ///< Internal list of proxy widgets in scene.
//    QList<QGraphicsProxyWidget *> fullScreenWidgets_; ///< List of full screen widgets.
};

#endif
