// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MainPanelButton.h"
#include "UiProxyWidget.h"

namespace CoreUi
{
    MainPanelButton::MainPanelButton(QWidget *parent, UiServices::UiProxyWidget *widget, const QString &widget_name) :
        QPushButton(widget_name, parent),
        controlled_widget_(widget),
        widget_name_(widget_name)
    {
        setFlat(true);
        setStyleSheet(QString("QPushButton { min-width: 50px; padding-left: 5px; padding-right: 5px; font-size: 11px;"
            "color: rgb(61, 80, 255); background-color: rgba(255, 255, 255, 0); }").append(
            "QPushButton::hover { color: black; background-color: rgba(255,255,255,150); border-radius: 5px; } QPushButton::!hover { color: rgb(255, 255, 255); }"
            "QPushButton::pressed { color: darkgrey; border: 0px; }"));
        setMaximumHeight(15);
        setMinimumHeight(15);
        QObject::connect(this, SIGNAL( clicked() ), this, SLOT( ToggleShow() )); 
    }

    MainPanelButton::~MainPanelButton()
    {
    }

    void MainPanelButton::ToggleShow()
    {
        if (controlled_widget_->isVisible())
            controlled_widget_->hide();
        else
            controlled_widget_->show();
    }

    void MainPanelButton::Hide()
    {
        controlled_widget_->hide();
    }

}
