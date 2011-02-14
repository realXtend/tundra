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

namespace Foundation
{
    class Framework;
}

class QGraphicsView;
class QRectF;

/// Light-weight UI service. Implements UiServiceInterface and provides means of  embedding Qt
/// widgets to the same scene/canvas as the 3D in-world view. Uses only one UI scene for everything.
class UiService : public UiServiceInterface
{
    Q_OBJECT

public:
    /// Constuctor.
    /** @param view The main graphics view.
    */
    explicit UiService(Foundation::Framework *framework, QGraphicsView *view);

    /// Destructor.
    ~UiService();

public slots:
    /// UiServiceInterface override.
    /** @note If you want to add full screen widget that is resized automatically to fit the screen
        when scene rectangle changes, remember to set Qt::FullScreen window state for the widget.
    */
    UiProxyWidget *AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog);

    UiProxyWidget *AddWidgetToSceneRaw(QWidget *widget, int flags = (int)Qt::Dialog) { return AddWidgetToScene(widget, (Qt::WindowFlags)flags); }

    /// UiServiceInterface override.
    bool AddWidgetToScene(UiProxyWidget *proxy);

    bool AddProxyWidgetToScene(UiProxyWidget *proxy);

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

    /// UiServiceInterface override. Does nothing.
    bool AddSettingsWidget(QWidget *widget, const QString &name) const { return false; }

    /// UiServiceInterface override. Returns the only UI scene we have.
    QGraphicsScene *GetScene(const QString &name) const { return scene_; }

    /// UiServiceInterface override.
    QWidget *LoadFromFile(const QString &file_path,  bool add_to_scene = true, QWidget *parent = 0);
    
    /// UiServiceInterface override.
    /// Does nothing.
    void RegisterScene(const QString &name, QGraphicsScene *scene) { }

    /// UiServiceInterface override. Does nothing.
    bool UnregisterScene(const QString &name) { return false; }

    /// UiServiceInterface override. Does nothing.
    bool SwitchToScene(const QString &name) { return false; }

    /// UiServiceInterface override.
    /// Does nothing.
    void RegisterUniversalWidget(const QString &name, QGraphicsProxyWidget *widget) {}
    
    /// UiServiceInterface override.
    /// Does nothing.
    void ShowNotification(CoreUi::NotificationBaseWidget *notification_widget) {}
    
  

private:
    /// Main graphics view.
    QGraphicsView *view_;

    /// Main graphics scene.
    QGraphicsScene *scene_;

    /// Used to access the Asset API for .ui file downloads.
    Foundation::Framework *framework_;

    /// Internal list of proxy widgets in scene.
    QList<QGraphicsProxyWidget *> widgets_;

    /// List of full screen widgets (Qt::WindowState::FullScreen on when embedded to the screen) in scene.
    QList<QGraphicsProxyWidget *> fullScreenWidgets_;

    /// Updates assets paths in Qt .ui-file so that all images etc. are found from our Asset-system. 
    /// @param data is actually Qt ui-file. 
    /// @return bytearray which data contains correct asset paths
    QByteArray UpdateAssetPaths(const QByteArray& data);


private slots:
    /// Remove proxywidget from internally maintained lists upon destruction.
    void OnProxyDestroyed(QObject *obj);

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
