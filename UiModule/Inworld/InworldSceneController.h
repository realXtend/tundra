// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_InworldSceneController_h
#define incl_UiModule_InworldSceneController_h

#include "UiModuleApi.h"
#include "UiModuleFwd.h"

#include <QObject>
#include <QList>

class QRectF;
class QGraphicsLinearLayout;

namespace Foundation { class Framework; }

namespace UiServices
{

    class UI_MODULE_API InworldSceneController : public QObject
    {
        
    Q_OBJECT

    public:
        //! Constructor.
        //! \param framework Framework pointer.
        //! \param ui_view UI view for this scene manager.
        InworldSceneController(Foundation::Framework *framework, QGraphicsView *ui_view);

        //! Destructor.
        ~InworldSceneController();

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

        //! Remove a proxy widget from scene if it exist there
        //! Used for removing your widget from scene. The show/hide toggle button will also be removed from the main panel.
        //! Note: Does not delete the proxy widget, after this is done its safe to delete your QWidget (this will delete the proxy also)
        //! \param widget Widget.
        void RemoveProxyWidgetFromScene(QWidget *widget);

        //! Brings the UiProxyWidget to front in the scene, set focus to it and shows it.
        //! \param widget Proxy widget.
        void BringProxyToFront(UiProxyWidget *widget);

        //! Brings the UiProxyWidget to front in the scene, set focus to it and shows it.
        //! \param widget Widget.
        void BringProxyToFront(QWidget *widget);

        //! Get the inworld ui scene
        QGraphicsScene *GetInworldScene() const { return inworld_scene_; }

        //! Set the current chat controller object to world chat widget
        void SetWorldChatController(QObject *controller);

        //! Set the im proxy to world chat widget for show/hide toggling
        void SetImWidget(UiProxyWidget *im_proxy);

        CoreUi::ControlPanelManager *GetControlPanelManager() { return control_panel_manager_; }

        QObject *GetSettingsObject();

    private:
        Q_DISABLE_COPY(InworldSceneController);

        //! Pointer to main QGraphicsView
        QGraphicsView *ui_view_;

        //! Pointer to inworld widget scene
        QGraphicsScene *inworld_scene_;

        //! Layout manager
        CoreUi::AnchorLayoutManager *layout_manager_;

        //! Menu manager
        CoreUi::MenuManager *menu_manager_;

        //! Control panel manager
        CoreUi::ControlPanelManager *control_panel_manager_;

        //! Core Widgets
        CoreUi::CommunicationWidget *communication_widget_;
        
        //! Internal list of UiProxyWidgets in scene
        QList<UiServices::UiProxyWidget *> all_proxy_widgets_in_scene_;

        //! Framework pointer.
        Foundation::Framework *framework_;

    private slots:
        //! Slot for applying new ui settings to all proxy widgets
        void ApplyNewProxySettings(int new_opacity, int new_animation_speed);

    };
}

#endif // incl_UiModule_InworldSceneController_h
