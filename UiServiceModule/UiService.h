/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiSceneService.h
 *  @brief  Light-weight UI service. Implements UiServiceInterface and provides 
 *          means of embedding Qt widgets to the same scene/canvas as the 3D in-world
 *          view. Uses only one UI scene for everything.
 */

#ifndef incl_UiServiceModule_UiService_h
#define incl_UiServiceModule_UiService_h

#include "UiServiceInterface.h"

class QGraphicsView;
class QRectF;

/*
 *  Light-weight UI service. Implements UiServiceInterface and provides 
 *  means of embedding Qt widgets to the same scene/canvas as the 3D in-world
 *  view. Uses only one UI scene for everything.
 */
class UiService : public Foundation::UiServiceInterface
{
    Q_OBJECT

public:
    /** Constuctor.
     *  @param view The main graphics view.
     */
    explicit UiService(QGraphicsView *view);

    /// Destructor.
    ~UiService();

public slots:
    /** UiServiceInterface override.
     *  @note If you want to add full screen widget that is resized automatically to fit the screen
     *  when scene rectangle changes, remember to set Qt::FullScreen window state for the widget.
     */
    UiProxyWidget *AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    /// UiServiceInterface override.
    bool AddWidgetToScene(UiProxyWidget *widget);

    /// UiServiceInterface override.
    void AddWidgetToMenu(QWidget *widget, const QString &name, const QString &menu, const QString &icon);

    /// UiServiceInterface override.
    void AddWidgetToMenu(UiProxyWidget *widget, const QString &name, const QString &menu, const QString &icon);

    /// UiServiceInterface override.
    void AddWidgetToMenu(QWidget *widget);

    /// UiServiceInterface override.
    void RemoveWidgetFromScene(QWidget *widget);

    /// UiServiceInterface override.
    void RemoveWidgetFromScene(QGraphicsProxyWidget *widget);

    /// UiServiceInterface override.
    void RemoveWidgetFromMenu(QWidget *widget);

    /// UiServiceInterface override.
    void RemoveWidgetFromMenu(QGraphicsProxyWidget *widget);

    /// UiServiceInterface override.
    void ShowWidget(QWidget *widget) const;

    /// UiServiceInterface override.
    void HideWidget(QWidget *widget) const;

    /// UiServiceInterface override.
    void BringWidgetToFront(QWidget *widget) const;

    /// UiServiceInterface override.
    void BringWidgetToFront(QGraphicsProxyWidget *widget) const;

    /// UiServiceInterface override.
    /// Does nothing.
    bool AddSettingsWidget(QWidget *widget, const QString &name) const { return false; }

    /// UiServiceInterface override.
    /// Returns the only UI scene we have.
    QGraphicsScene *GetScene(const QString &name) const { return scene_; }

    /// UiServiceInterface override.
    /// Does nothing.
    void RegisterScene(const QString &name, QGraphicsScene *scene) { }

    /// UiServiceInterface override.
    /// Does nothing.
    bool UnregisterScene(const QString &name) { return false; }

    /// UiServiceInterface override.
    /// Does nothing.
    bool SwitchToScene(const QString &name) { return false; }

private:
    /// Main graphics view.
    QGraphicsView *view_;

    /// Main graphics scene.
    QGraphicsScene *scene_;

    /// Internal list of proxy widgets in scene.
    QList<QGraphicsProxyWidget *> widgets_;

    /// List of full screen widgets (Qt::WindowState::FullScreen on when embedded to the screen) in scene.
    QList<QGraphicsProxyWidget *> fullScreenWidgets_;

private slots:
    /** Performs different operations for proxy widgets when scene rectangle is changed, f.ex. resizes
     *  full screen widgets to fit the screen.
     *  @param rect New scene rectangle.
     */
    void SceneRectChanged(const QRectF &rect);

    /** Deletes widget and the corresponding proxy widget if widget has WA_DeleteOnClose on.
     *  The caller of this slot is retrieved by using QObject::sender().
     */
    void DeleteCallingWidgetOnClose();
};

#endif
