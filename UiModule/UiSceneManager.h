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
        bool AddSettingsWidget(QWidget *settings_widget, const QString &tab_name);

        //! Adds a Qt Widget to the current scene, returns the added QGraphicsProxyWidget.
        //! Conviniance function if you dont want to bother and define your UiWidgetProperties.
        //! \param widget QWidget to be added to the scene.
        //! \return UiProxyWidget if succesfull, otherwise 0
        UiProxyWidget* AddWidgetToScene(QWidget *widget);

        //! Adds a Qt Widget to the current scene with Naali widget properties, returns the added QGraphicsProxyWidget
        //! \param widget QWidget to be added to the scene.
        //! \param widget_properties Properties for the widget.
        //! \return UiProxyWidget if succesfull, otherwise 0
        UiProxyWidget* AddWidgetToScene(QWidget *widget, const UiServices::UiWidgetProperties &widget_properties);

        //! Adds a already created UiProxyWidget into the scene.
        //! Please prefer using AddWidgetToScene() with normal QWidget 
        //! and properties instead of this directly.
        //! \param widget Proxy widget.
        bool AddProxyWidget(UiServices::UiProxyWidget *proxy_widget);

        //! Remove a proxy widget from scene if it exist there
        //! Used for removing your widget from scene. The show/hide toggle button will also be removed from the main panel.
        //! Note: Does not delete the proxy widget, after this is done its safe to delete your QWidget (this will delete the proxy also)
        //! \param widget Proxy widget.
        void RemoveProxyWidgetFromScene(UiServices::UiProxyWidget *proxy_widget);

        //! Get proxy widget pointer by name
        //! \return Pointer to the proxy widget or 0 if not found.
        UiProxyWidget *GetProxyWidget(const QString &widget_name);

        //! Brings the UiProxyWidget to front in the scene and set focus to it
        void BringProxyToFront(UiProxyWidget *widget);

        ///\note
        /// THE FOLLOWING FUNCTIONS SHOULD BE USED BY THE CORE UI ONLY (INTERNALLY BY UI MODULE). DON'T USE THESE DIRECTLY IF
        /// YOU ARE NOT 100% SURE WHAT YOU ARE DOING.

        //! Get the inworld controls
        //! \return The main panel pointer or 0 if panel does not exist yet
        CoreUi::MainPanel *GetMainPanel() const;

        //! Inits the ui for connected state
        void Connected();

        //! Inits the ui for disconnected state
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
        UiProxyWidget *login_proxy_widget_;
        UiProxyWidget *main_panel_proxy_widget_;
        UiProxyWidget *settings_widget_proxy_widget_;

        //! Framework pointer.
        Foundation::Framework *framework_;

    private slots:
        //! Slot for applying new ui settings to all proxy widgets
        void ApplyNewProxySettings(int new_opacity, int new_animation_speed);

        //! Slot for keeping full screen/layout core widgets properly sized
        void SceneRectChanged(const QRectF &new_scene_rect);

    signals:
        //! Emits when connected for modules to utilise
        void UiStateChangeConnected();

        //! Emits when disconnected for modules to utilise
        void UiStateChangeDisconnected();
    };
}

#endif // incl_UiModule_UiSceneManager_h
