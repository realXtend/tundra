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
    MainPanel::MainPanel() :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget())
    {
        Initialise();
    }

    void MainPanel::Initialise()
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);
    }

    // Public services

    MainPanelButton *MainPanel::SetSettingsWidget(UiServices::UiProxyWidget *settings_widget, const QString &widget_name)
    {
        MainPanelButton *control_button = new MainPanelButton(internal_widget_, settings_widget, QString(" " + widget_name));
        topcontrols->insertWidget(0, control_button);
        return control_button;
    }
}
