/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiServiceInterface.h
 *  @brief  Interface for Naali's user interface ulitizing Qt's QWidgets.
 *          If you want to see your QWidgets external to the main application just call show() for them.
 */

#ifndef incl_Interfaces_UiServiceInterface_h
#define incl_Interfaces_UiServiceInterface_h

#include "ServiceInterface.h"

#include <QObject>

class QWidget;
class QGraphicsProxyWidget; 
class QGraphicsScene;

class UiProxyWidget;

namespace CoreUi
{
    class NotificationBaseWidget;
}

/// Interface for Naali's user interface ulitizing Qt's QWidgets.
/** If you want to see your QWidgets external to the main application just call show() for them.
*/
class UiServiceInterface : public QObject, public Foundation::ServiceInterface
{
    Q_OBJECT

public:
    /// Default constructor.
    UiServiceInterface() {}

    /// Destructor.
    virtual ~UiServiceInterface() {}

public slots:
    /** Adds widget to scene.
     *  @param  widget Widget.
     *  @param  flags Window flags. Qt::Dialog is used as default.
     *          It creates movable proxy widget which has title bar and frames.
     *          If you want add widget without title bar and frames, use Qt::Widget.
     *          For further information, see http://doc.qt.nokia.com/4.6/qt.html#WindowType-enum
     *  @return Proxy widget of the added widget.
     */
    virtual UiProxyWidget *AddWidgetToScene(QWidget *widget, Qt::WindowFlags flags = Qt::Dialog) = 0;

    /** Adds user-created UiProxyWidget to the scene.
     *  @param widget Proxy widget.
     */
    virtual bool AddWidgetToScene(UiProxyWidget *widget) = 0;

    /** Adds widget to menu without any spesific properties: adds entry to the root menu,
     *  takes name from the window title and uses default icon.
     *  @param widget Widget.
     *
     *  @note Doesn't add the widget to the scene.
     */
    virtual void AddWidgetToMenu(QWidget *widget) = 0;

    /** Adds widget to menu.
     *  @param widget Widget.
     *  @param name Name of the menu entry.
     *  @param menu Name of the menu. If the menu doesn't exist, it is created. If no name is given the entry is added to the root menu.
     *  @param icon Path to image which will be used as the icon for the entry. If no path is given, default icon is used.
     *
     *  @note Doesn't add the widget to the scene.
     */
    virtual void AddWidgetToMenu(QWidget *widget, const QString &entry, const QString &menu = "", const QString &icon = "") = 0;

    /** This is an overloaded function.
     *  @param widget Proxy widget.
     *  @param name Name of the menu entry.
     *  @param menu Name of the menu. If the menu doesn't exist, it is created. If no name is given the entry is added to the root menu.
     *  @param icon Path to image which will be used as the icon for the entry. If no path is given, default icon is used.
     *
     *  @note Doesn't add the widget to the scene.
     */
    virtual void AddWidgetToMenu(UiProxyWidget *widget, const QString &name, const QString &menu = "", const QString &icon = "") = 0;

    /** Removes widget from the scene.
     *  @param widget Widget.
     */
    virtual void RemoveWidgetFromScene(QWidget *widget) = 0;

    /** This is an overloaded function.
     *  @param widget Proxy widget.
     */
    virtual void RemoveWidgetFromScene(QGraphicsProxyWidget *widget) = 0;

    /** Removes widget from menu.
     *  @param widget The controlled widget.
     */
    virtual void RemoveWidgetFromMenu(QWidget *widget) = 0;

    /** This is an overloaded function.
     *  @param widget The controlled widget.
     */
    virtual void RemoveWidgetFromMenu(QGraphicsProxyWidget *widget) = 0;

    /** Shows the widget's proxy widget in the scene.
     *  @param widget Widget.
     */
    virtual void ShowWidget(QWidget *widget) const = 0;

    /** Hides the widget's proxy widget in the scene.
     *  @param widget Widget.
     */
    virtual void HideWidget(QWidget *widget) const = 0;

    /** Brings the widget's proxy widget to front in the and sets focus to it.
     *  @param widget Widget.
     */
    virtual void BringWidgetToFront(QWidget *widget) const = 0;

    /** This is an overloaded function.
     *  Brings the proxy widget to front in the scene and sets focus to it.
     *  @param widget Proxy widget.
     */
    virtual void BringWidgetToFront(QGraphicsProxyWidget *widget) const = 0;

    /** Adds a setting widget the UI's main settings widget (if applicable).
     *  @param widget Settings widget.
     *  @param name Preferred name widget
     *  @return true if widget was added successfully, false otherwise.
     */
    virtual bool AddSettingsWidget(QWidget *widget, const QString &name) const = 0;

    /** Returns scene with the requested name.
     *  @param name Name of the scene.
     *  @return Graphic scene with the requested name, or null if not found.
     */
    virtual QGraphicsScene *GetScene(const QString &name) const = 0;

    /** Registers new scene.
     *  The instance which creates new scene is also responsible for its deletion.
     *  @param name Name of the scene.
     *  @param scene Graphics scene.
     *  @sa UnregisterScene.
     */
    virtual void RegisterScene(const QString &name, QGraphicsScene *scene) = 0;

    /** Unregisters graphics scene.
     *  @param name Name of the scene.
     *  @return True if the scene was found and deleted succesfully, false otherwise.
     *  @note Does not delete the scene.
     */
    virtual bool UnregisterScene(const QString &name) = 0;

    /** Switches the active scene.
     *  @param name Name of the scene.
     *  @return True if the scene existed and was activate ok, false otherwise.
     */
    virtual bool SwitchToScene(const QString &name) = 0;

    /** Registers a universal widget. This means when scene is changed all interested will get
     *  a TranferRequest signal
     *  @param name Name of the widget
     *  @param widget The registered QWidget
     */
    virtual void RegisterUniversalWidget(const QString &name, QGraphicsProxyWidget *widget) = 0;

    /** Request notification manager to show given notificaiton widget
        @todo move NotificationBaseWidget class to an public interface.
     */
    virtual void ShowNotification(CoreUi::NotificationBaseWidget *notification_widget) = 0;

    /** Load widget from .ui file and as default add it to a scene. This method is for scripters.
     *  @param file_path ui file location.
     *  @param parent Pointer to parent widget.
     *  @param add_to_scene do we want to add new widget to scene.
     *  @return loaded widget's pointer (null if fail to load).
     */
    virtual QWidget *LoadFromFile(const QString &file_path, bool add_to_scene = true, QWidget *parent = 0) = 0;

signals:
    /** Emitted when scene is changed.
     *  @param oldName Name of the old scene.
     *  @param newName Name of the new scene.
     */
    void SceneChanged(const QString &oldName, const QString &newName);

    /** Emitted when scene changes for every widget that has registered
     *  to be a universal widget. Scenes have to implement to catch this if they accept widgets from other scenes.
     *  @param widget_name The tranfering widgets name
     *  @param widget The transfering widget
     */
    void TransferRequest(const QString &widget_name, QGraphicsProxyWidget *widget);

    /** Emitted when ShowNotification get called.
        @param message The textual message of NotificationBaseWidget showed
     */  
    void Notification(const QString &message);
};

#endif
