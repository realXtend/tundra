// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "SettingsWidget.h"

namespace CoreUi
{
    SettingsWidget::SettingsWidget()
        : QWidget()
    {
        setting_ui_.setupUi(this);
        setting_ui_.settingsTabWidget->clear();
    }

    SettingsWidget::~SettingsWidget()
    {

    }

    void SettingsWidget::AddWidget(QWidget *widget, const QString &tab_name)
    {
        setting_ui_.settingsTabWidget->addTab(widget, tab_name);
    }
}