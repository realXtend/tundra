// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_InworldSceneController_h
#define incl_UiModule_InworldSceneController_h

#include "UiModuleApi.h"
#include "UiModuleFwd.h"

#include <QObject>
#include <QList>
#include <QRectF>

#include "ui_TeleportWidget.h"

class QRectF;
class QGraphicsLinearLayout;

namespace Foundation
{
    class Framework;
}

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
        bool AddSettingsWidget(QWidget *settings_widget, const QString &tab_name) const;

        //! Adds received region names to TeleportWidget
        //! \param region_names Region names to be added to widget
        void SetTeleportWidget(const std::vector<std::string> &region_names);

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
        void BringProxyToFront(UiProxyWidget *widget) const;

        //! Brings the UiProxyWidget to front in the scene, set focus to it and shows it.
        //! \param widget Widget.
        void BringProxyToFront(QWidget *widget) const;

        //! Shows the UiProxyWidget of QWidget in the scene.
        //! \param widget Widget.
        void ShowProxyForWidget(QWidget *widget) const;

        //! Hides the UiProxyWidget of QWidget in the scene.
        //! \param widget Widget.
        void HideProxyForWidget(QWidget *widget) const;

        //! Get the inworld ui scene
        QGraphicsScene *GetInworldScene() const { return inworld_scene_; }

        //! Please dont call this if you dont know what you are doing
        //! Set the current chat controller object to world chat widget
        void SetWorldChatController(QObject *controller) const;

        //! Please dont call this if you dont know what you are doing
        //! Set the im proxy to world chat widget for show/hide toggling
        void SetImWidget(UiProxyWidget *im_proxy) const;

        //void SetCommunicationWidget(UiProxyWidget *im_proxy);

        //! Set focus to chat line edit
        void SetFocusToChat() const;

        //! Get ControlPanelManager pointer
        CoreUi::ControlPanelManager *GetControlPanelManager()  const { return control_panel_manager_; }

        //! Get SettingsWidget QObject pointer to make save/cancel connections outside UiModule
        QObject *GetSettingsObject() const;

		//Applying new proxy position
		void ApplyNewProxyPosition(const QRectF &new_rect);

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
	
	//Store last scene rectangle 
	QRectF last_scene_rect;

    private slots:
        //! Slot for applying new ui settings to all proxy widgets
        void ApplyNewProxySettings(int new_opacity, int new_animation_speed) const;
    };
}

#endif // incl_UiModule_InworldSceneController_h
