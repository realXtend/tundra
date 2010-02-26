// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "CoreDefines.h"
#include "MainPanel.h"
#include "MainPanelButton.h"
#include "ConfigurationManager.h"
#include "Login/InworldLoginDialog.h"
#include "UiProxyWidget.h"
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
        inworld_login_dialog_(new CoreUi::InworldLoginDialog(framework)),
        panel_widget_(0)
    {
        initialize_();
    }

    MainPanel::~MainPanel()
    {
        SAFE_DELETE(inworld_login_dialog_);
        SAFE_DELETE(panel_widget_);
        all_proxy_widgets_.clear();
    }

    void MainPanel::initialize_()
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
            navigate_toggle_button_ = panel_widget_->findChild<QPushButton *>("navigate_button");
            navigate_connect_button_ = panel_widget_->findChild<QPushButton *>("navigate_connect");
            navigate_address_bar_ = panel_widget_->findChild<QComboBox *>("navigate_address");

            navigate_frame_->hide();
            panel_widget_->setMaximumHeight(25);

            connect(logout_button, SIGNAL( clicked() ), this, SLOT( HideWidgets() ));
            connect(navigate_toggle_button_, SIGNAL( clicked() ), this, SLOT( ToggleNavigation() ));
            connect(navigate_address_bar_->lineEdit(), SIGNAL( returnPressed() ), this, SLOT( Navigate() ));
            connect(navigate_connect_button_, SIGNAL( clicked() ), this, SLOT( Navigate() ));
            connect(inworld_login_dialog_, SIGNAL( TryLogin(QMap<QString,QString> &) ), this, SLOT( ParseAndEmitLogin(QMap<QString,QString> &) ));

            InitBookmarks();
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

    // Private functions

    void MainPanel::InitBookmarks()
    {
        int index = 0;
        QString value("none");
        QString configGroup("WorldBookmarks");
        QString configKey;
        QStringList bookmarks = QStringList("Drop down this menu for your bookmarks");

        while (value != "")
        {
            configKey = "world";
            ++index;
            value = QString(framework_->GetDefaultConfigPtr()->GetSetting<std::string>(
                configGroup.toStdString(), configKey.append(QString::number(index)).toStdString()).c_str());
            if (!value.isEmpty() && !value.isNull())
                bookmarks.append(value);
        }

        bookmark_next_index_ = index;

        if (bookmarks.count() == 1)
            navigate_address_bar_->addItem("Input a world address that you would like to connect to");
        else
            navigate_address_bar_->addItems(bookmarks);
    }

    void MainPanel::Navigate()
    {
        // Do some world url validating
        QString world_address(navigate_address_bar_->lineEdit()->text());
        if (!world_address.startsWith("Input a world address that") && 
            !world_address.startsWith("You must input a valid world address") &&
            !world_address.startsWith("Server address ") &&
            !world_address.startsWith("Drop down this menu for you") &&
            !world_address.isEmpty())
        {
            if (!world_address.startsWith("http://"))
                world_address = "http://" + world_address;
            QUrl world_url(world_address);
            if (world_url.port() == -1)
                world_url.setPort(9000);

            // If valid lets ask for credentials
            if (world_url.isValid())
                inworld_login_dialog_->ShowModalDialog(world_url.toString());
            else
                navigate_address_bar_->lineEdit()->setText(QString("Server address %1 was not a valid url").arg(world_url.toString()));
        }
        else
            navigate_address_bar_->lineEdit()->setText("You must input a valid world address first before connecting");
    }

    void MainPanel::HideWidgets()
    {
        foreach (UiServices::UiProxyWidget *widget, all_proxy_widgets_)
            widget->hide();
    }

    void MainPanel::ToggleNavigation()
    {
        if (navigating_)
        {
            navigate_toggle_button_->setStyleSheet("QPushButton#navigate_button { padding: 0px; padding-right: 5px;"
                "min-width: 17px; min-height: 16px; background-position: top left; background-repeat: no-repeat;"
                "background-image: url(':/images/arrow_down.png'); } QPushButton#navigate_button:hover { background-image: url(':/images/arrow_down_hover.png'); }");
            panel_widget_->setMaximumHeight(25);
            navigate_frame_->hide();
        }
        else
        {
            navigate_toggle_button_->setStyleSheet("QPushButton#navigate_button { padding: 0px; padding-right: 5px;"
                "min-width: 17px; min-height: 16px; background-position: top left; background-repeat: no-repeat;"
                "background-image: url(':/images/arrow_up.png'); } QPushButton#navigate_button:hover { background-image: url(':/images/arrow_up_hover.png'); }");
            panel_widget_->setMaximumHeight(50);
            navigate_frame_->show();
        }

        navigating_ = !navigating_;
    }

    void MainPanel::ParseAndEmitLogin(QMap<QString,QString> &input_map)
    {
        if (input_map["AuthType"] == "OpenSim")
        {
            emit CommandLoginOpenSim(input_map["Username"], input_map["Password"], input_map["WorldAddress"]);
        }
        else if (input_map["AuthType"] == "RealXtend")
        {
            emit CommandLoginRealxtend(input_map["Username"], input_map["Password"], input_map["AuthenticationAddress"], input_map["WorldAddress"]);
        }
        else
            return;

        panel_widget_->setMaximumHeight(25);
        navigate_frame_->hide();

        QString configGroup("WorldBookmarks");
        QString configKey(QString("world%1").arg(QString::number(bookmark_next_index_)));

        if ( framework_->GetConfigManager()->HasKey(configGroup.toStdString(), configKey.toStdString()) )
            framework_->GetConfigManager()->SetSetting<std::string>(configGroup.toStdString(), configKey.toStdString(), input_map["WorldAddress"].toStdString());
        else
            framework_->GetConfigManager()->DeclareSetting<std::string>(configGroup.toStdString(), configKey.toStdString(), input_map["WorldAddress"].toStdString());
        
        framework_->GetConfigManager()->Export();
    }
}
