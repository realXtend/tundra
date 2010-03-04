// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_MainPanel_h
#define incl_UiModule_MainPanel_h

#include "SettingsWidget.h"

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QString>
#include <QObject>
#include <QFrame>
#include <QComboBox>

namespace CoreUi
{
    class MainPanelButton;
}

namespace Foundation
{
    class Framework;
}

namespace UiServices
{
    class UiProxyWidget;
}

namespace CoreUi
{
    class MainPanel : public QObject
    {
        Q_OBJECT

    public:
        explicit MainPanel(Foundation::Framework *framework);
        virtual ~MainPanel();

        //! Adds a widget to the main panel.
        /// @param UiProxyWidget
        /// @param QString widget name
        /// @return MainPanelButton
        MainPanelButton *AddWidget(UiServices::UiProxyWidget *widget, const QString &widget_name);

        //! Adds the CoreUi settings widget to this bar. Done by scenemanager.
        /// @param SettingsWidget
        /// @param QString Widget name
        /// @return MainPanelButton
        MainPanelButton *SetSettingsWidget(UiServices::UiProxyWidget *settings_widget, const QString &widget_name);

        //! Removes a widget from the main panel
        /// @param UiProxyWidget
        bool RemoveWidget(UiServices::UiProxyWidget *widget);

        //! Returns the QWidget of the ui. Retrieved by scene manager to insert into layout when inworld.
        QWidget *GetWidget() { return panel_widget_; }

        //! Returns the list that contains all the proxy widgets.
        QList<UiServices::UiProxyWidget *> GetProxyWidgetList() { return all_proxy_widgets_; }

        QPushButton *logout_button;
        QPushButton *quit_button;

    private:
        QWidget *panel_widget_;
        QHBoxLayout *layout_;
        QHBoxLayout *topcontrols_;
        QList<UiServices::UiProxyWidget *> all_proxy_widgets_;

        //! Navigation frame of this widget and its elements
        QFrame *navigate_frame_;
        QPushButton *navigate_toggle_button_;
        QPushButton *navigate_connect_button_;
        QComboBox *navigate_address_bar_;
        bool navigating_;
        int bookmark_next_index_;

        //! Pointer to framework for configs
        Foundation::Framework *framework_;

        //! Settings Widget
        SettingsWidget settings_widget_;

    private slots:
        void Initialise();
        void HideWidgets();

    };
}

#endif // incl_UiModule_MainPanelControls_h
