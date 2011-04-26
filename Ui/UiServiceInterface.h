/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiServiceInterface.h
 *  @note   THIS CLASS IS DEPRECATD AND WILL BE REMOVED.
 */

#ifndef incl_Ui_UiServiceInterface_h
#define incl_Ui_UiServiceInterface_h

#include "IService.h"

#include <QObject>

class QWidget;
class QGraphicsProxyWidget;
class QGraphicsScene;

class UiProxyWidget;

namespace CoreUi
{
    class NotificationBaseWidget;
}

/** @todo THIS CLASS IS DEPRECATD AND WILL BE REMOVED.
    @note THIS CLASS IS DEPRECATD AND WILL BE REMOVED.
*/
class UiServiceInterface : public QObject, public IService
{
    Q_OBJECT

public:
    UiServiceInterface() {}
    virtual ~UiServiceInterface() {}

public slots:
    virtual UiProxyWidget *AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog) = 0;
    virtual bool AddWidgetToScene(UiProxyWidget *widget) = 0;
    virtual void AddWidgetToMenu(QWidget *widget) = 0;
    virtual void AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu = "", const QString &icon = "") = 0;
    virtual void AddWidgetToMenu(UiProxyWidget *widget, const QString &name, const QString &menu = "", const QString &icon = "") = 0;
    virtual void RemoveWidgetFromScene(QWidget *widget) = 0;
    virtual void RemoveWidgetFromScene(QGraphicsProxyWidget *widget) = 0;
    virtual void RemoveWidgetFromMenu(QWidget *widget) = 0;
    virtual void RemoveWidgetFromMenu(QGraphicsProxyWidget *widget) = 0;
    virtual void ShowWidget(QWidget *widget) const = 0;
    virtual void HideWidget(QWidget *widget) const = 0;
    virtual void BringWidgetToFront(QWidget *widget) const = 0;
    virtual void BringWidgetToFront(QGraphicsProxyWidget *widget) const = 0;
    virtual bool AddSettingsWidget(QWidget *widget, const QString &name) const = 0;
    virtual QGraphicsScene *GetScene(const QString &name) const = 0;
    virtual void RegisterScene(const QString &name, QGraphicsScene *scene) = 0;
    virtual bool UnregisterScene(const QString &name) = 0;
    virtual bool SwitchToScene(const QString &name) = 0;
    virtual void RegisterUniversalWidget(const QString &name, QGraphicsProxyWidget *widget) = 0;
    virtual void ShowNotification(CoreUi::NotificationBaseWidget *notification_widget) = 0;

signals:
    void SceneChanged(const QString &oldName, const QString &newName);
    void TransferRequest(const QString &widget_name, QGraphicsProxyWidget *widget);
    void Notification(const QString &message);
};

#endif
