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
        {
            controlled_widget_->hide();
            setStyleSheet(QString("QPushButton { color: white; background-color: rgba(255,255,255,0); border-radius: 5px; } QPushButton::hover { color: black; background-color: rgba(255,255,255,160); } QPushButton::pressed { color: darkgrey; border: 0px; }"));
        }
        else
        {
            controlled_widget_->show();
            setStyleSheet(QString("QPushButton { color: black; background-color: rgba(255,255,255,125); border-radius: 5px; } QPushButton::hover { color: black; background-color: rgba(255,255,255,160); } QPushButton::pressed { color: darkgrey; border: 0px; }"));
        }
    }

    void MainPanelButton::ControlledWidgetHidden()
    {
        setStyleSheet(QString("QPushButton { color: white; background-color: rgba(255,255,255,0); border-radius: 5px; } QPushButton::hover { color: black; background-color: rgba(255,255,255,160); } QPushButton::pressed { color: darkgrey; border: 0px; }"));
    }

}
