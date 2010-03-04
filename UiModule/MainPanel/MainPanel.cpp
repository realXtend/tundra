// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CoreDefines.h"
#include "MainPanel.h"
#include "MainPanelButton.h"
#include "ConfigurationManager.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Framework.h"

#include <QUiLoader>
#include <QFile>
#include <QUrl>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    MainPanel::MainPanel(Foundation::Framework *framework) :
        QObject(),
        framework_(framework),
        navigating_(false),
        panel_widget_(0)
    {
        Initialise();
    }

    MainPanel::~MainPanel()
    {
        SAFE_DELETE(panel_widget_);
        all_proxy_widgets_.clear();
    }

    void MainPanel::Initialise()
    {
        QUiLoader loader;
        QFile uiFile("./data/ui/mainpanel.ui");
        if (uiFile.exists())
        {
            panel_widget_ = loader.load(&uiFile);
            panel_widget_->hide();

            layout_ = panel_widget_->findChild<QHBoxLayout *>("contolBarLayout");
            topcontrols_ = panel_widget_->findChild<QHBoxLayout *>("topcontrols");
            logout_button = panel_widget_->findChild<QPushButton *>("buttonLogout");
            quit_button = panel_widget_->findChild<QPushButton *>("buttonQuit");

            navigate_frame_ = panel_widget_->findChild<QFrame *>("navigation_frame");

            navigate_frame_->hide();
            panel_widget_->setMaximumHeight(25);

            connect(logout_button, SIGNAL( clicked() ), this, SLOT( HideWidgets() ));
        }
    }

    // Public services

    MainPanelButton *MainPanel::AddWidget(UiServices::UiProxyWidget *widget, const QString &widget_name)
    {
        if (all_proxy_widgets_.indexOf(widget) == -1)
        {
            MainPanelButton *control_button = new MainPanelButton(panel_widget_, widget, QString(" " + widget_name));
            layout_->addWidget(control_button);
            all_proxy_widgets_.append(widget);
            return control_button;
        }
        return 0;
    }

    MainPanelButton *MainPanel::SetSettingsWidget(UiServices::UiProxyWidget *settings_widget, const QString &widget_name)
    {
        if (all_proxy_widgets_.indexOf(settings_widget) == -1)
        {
            MainPanelButton *control_button = new MainPanelButton(panel_widget_, settings_widget, QString(" " + widget_name));
            topcontrols_->insertWidget(2, control_button);
            all_proxy_widgets_.append(settings_widget);
            return control_button;
        }
        return 0;
    }

    bool MainPanel::RemoveWidget(UiServices::UiProxyWidget *widget)
    {
        int found_index = all_proxy_widgets_.indexOf(widget);
        if (found_index != -1)
        {
            // Remove from widget list
            all_proxy_widgets_.removeAt(found_index);

            // Remove from toolbar and delete button
            MainPanelButton *control_button = widget->GetControlButton();
            int button_index = layout_->indexOf(control_button);
            if (button_index != -1)
            {
                layout_->removeItem(layout_->itemAt(button_index));
                SAFE_DELETE(control_button);
                widget->SetControlButton(0); // To be sure its not used internally after this
            }
            return true;
        }
        else
            return false;
    }

    void MainPanel::HideWidgets()
    {
        foreach (UiServices::UiProxyWidget *widget, all_proxy_widgets_)
            widget->hide();
    }
}
