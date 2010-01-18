// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_UiSceneManager_h
#define incl_UiModule_UiSceneManager_h

#include "UiModuleApi.h"

#include <Foundation.h>

#include <QObject>
#include <QList>

class QRectF;
class QGraphicsLinearLayout;
class QGraphicsWidget;

namespace CoreUi
{
    class MainPanel;
    class SettingsWidget;
}

namespace UiServices
{
    class UiProxyWidget;
    class UiWidgetProperties;

    class UI_MODULE_API UiSceneManager : public QObject
    {
        Q_OBJECT

    public:
        //! Constructor.
        //! \param framework Framework pointer.
        //! \param ui_view UI view for this scene manager.
        UiSceneManager(Foundation::Framework *framework, QGraphicsView *ui_view);

        //! Destructor.
        ~UiSceneManager();

    public slots:
        //! Adds a Qt Widget to the settings widget as its own tab
        //! \param widget QWidget to be added to the settings widget
        //! \param tab_name QString name of the tab shown in widget
        //! \return trued if add succesfull, false otherwise
        bool UiSceneManager::AddSettingsWidget(QWidget *settings_widget, const QString &tab_name);

        //! Adds a Qt Widget to the current scene, returns the added QGraphicsProxyWidget.
        //! The caller of this function is the owner of the proxy widget.
        //! \param widget QWidget to be added to the scene.
        UiProxyWidget* AddWidgetToCurrentScene(QWidget *widget);

        //! Adds a Qt Widget to the current scene with Naali widget properties, returns the added QGraphicsProxyWidget
        //! \param widget QWidget to be added to the scene.
        //! \param widget_properties Properties for the widget.
        UiProxyWidget* AddWidgetToCurrentScene(QWidget *widget, const UiServices::UiWidgetProperties &widget_properties);

        //! Adds a already created UiProxyWidget into the scene, used for 3D to canvas swaps
        //! \param widget Proxy widget.
        bool AddProxyWidget(UiServices::UiProxyWidget *widget);

        //! Remove a proxy widget from scene if it exist there
        //! Doesn't delete the widget, only removes it from the graphics scene.
        //! \param widget Proxy widget.
        void RemoveProxyWidgetFromCurrentScene(UiProxyWidget *widget);

        //! Get the inworld controls
        CoreUi::MainPanel *GetMainPanel() const { return main_panel_; }

        //! Gets all present proxy widgets from scene
        QList<UiProxyWidget *> GetAllProxyWidgets();

        //! Get proxy widget pointer by name
        //! \return Pointer to the widget, or null if not found.
        UiProxyWidget *GetProxyWidget(const QString &widget_name);

        //! Slot for keeping full screen core widgets properly sized
        void SceneRectChanged(const QRectF &new_scene_rect);

        //! Inits the ui for connected state
        void Connect();

        //! Inits the ui for disconnected state
        void Disconnect();

        //! Brings the UiProxyWidget on to front in the scene and set focus to it
        void BringToFront(UiProxyWidget *widget);

    signals:
        //! Emits when connected for modules to utilise
        void Connected();

        //! Emits when disconnected for modules to utilise
        void Disconnected();

    private:
        Q_DISABLE_COPY(UiSceneManager);

        //! Inits the full screen widget and its layout
        void InitMasterLayout();

        //! Removes all widgets from the full screen widget layout
        void ClearContainerLayout();

        //! Pointer to main QGraphicsView
        QGraphicsView *ui_view_;

        //! Main layout for full screen widgets
        QGraphicsLinearLayout *container_layout_;

        //! Bottom container widget for main layout
        QGraphicsWidget *container_widget_;

        //! CoreUi Widgets
        CoreUi::MainPanel *main_panel_;
        CoreUi::SettingsWidget *settings_widget_;

        //! Proxy widgets
        UiProxyWidget *login_widget_;
        UiProxyWidget *main_panel_proxy_widget_;
        UiProxyWidget *settings_widget_proxy_widget_;

        //! Framework pointer.
        Foundation::Framework *framework_;
    };
}

#endif // incl_UiModule_UiSceneManager_h
