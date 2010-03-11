// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_MainPanel_h
#define incl_UiModule_MainPanel_h

#include "SettingsWidget.h"
#include "ui_MainPanelWidget.h"

#include <QGraphicsProxyWidget>

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
    class MainPanel : public QGraphicsProxyWidget, public Ui::MainPanelWidget
    {
        Q_OBJECT

    public:
        MainPanel();

        //! Adds the CoreUi settings widget to this bar. Done by InworldSceneController.
        /// @param SettingsWidget
        /// @param QString Widget name
        /// @return MainPanelButton
        MainPanelButton *SetSettingsWidget(UiServices::UiProxyWidget *settings_widget, const QString &widget_name);

    private:
        //! Internal QWidget
        QWidget *internal_widget_;

        //! Settings Widget
        SettingsWidget settings_widget_;

    private slots:
        void Initialise();
    };
}

#endif // incl_UiModule_MainPanelControls_h
