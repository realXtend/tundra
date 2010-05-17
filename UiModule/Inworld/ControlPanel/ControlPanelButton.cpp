// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ControlPanelButton.h"

namespace CoreUi
{
    ControlPanelButton::ControlPanelButton(UiDefines::ControlButtonType type) :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget()),
        type_(type)
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);
        connect(actionButton, SIGNAL(clicked()), SLOT(Clicked()));

        InitStyle();
    }

    // Public

    qreal ControlPanelButton::GetContentWidth()
    {
        return internal_widget_->width();
    }

    UiDefines::ControlButtonType ControlPanelButton::GetType()
    {
        return type_;
    }

    void ControlPanelButton::CheckStyle(bool item_visible)
    {
        QString normal_image = "./data/ui/images/menus/";
        switch (type_)
        {
            case UiDefines::Notifications:
                if (item_visible)
                    normal_image += "uibutton_NOTIF_click.png";
                else
                    normal_image += "uibutton_NOTIF_normal.png";
                break;

            case UiDefines::Settings:
                if (item_visible)
                    normal_image += "uibutton_SET_click.png";
                else
                    normal_image += "uibutton_SET_normal.png";
                break;

            case UiDefines::Teleport:
                if (item_visible)
                    normal_image += "uibutton_TEL_click.png";
                else
                    normal_image += "uibutton_TEL_normal.png";
                break;

            default:
                return;
        }

        actionButton->setStyleSheet("QPushButton { background-color: transparent; border: 0px; background-position: top left;"
                                    "background-repeat: no-repeat; background-image: url('" + normal_image + "'); }");
    }

    // Private

    void ControlPanelButton::InitStyle()
    {
        QString location = "./data/ui/images/menus/";
        QString normal_image = location;
        QString hover_image = location;
        QString clicked_image = location;
        QSize fixed_size;

        switch (type_)
        {
            case UiDefines::Ether:
            {
                normal_image += "uibutton_ETHER_normal.png";
                hover_image += "uibutton_ETHER_hover.png";
                clicked_image += "uibutton_ETHER_click.png";
                fixed_size = QSize(100,32);
                internal_widget_->setContentsMargins(0,0,0,0);
                break;
            }
            case UiDefines::Quit:
            {
                normal_image += "uibutton_QUIT_normal.png";
                hover_image += "uibutton_QUIT_hover.png";
                clicked_image += "uibutton_QUIT_click.png";
                fixed_size = QSize(76,32);
                internal_widget_->setContentsMargins(0,0,10,0);
                break;
            }
            case UiDefines::Settings:
            {
                normal_image += "uibutton_SET_normal.png";
                hover_image += "uibutton_SET_hover.png";
                clicked_image += "uibutton_SET_click.png";
                fixed_size = QSize(62,23);
                internal_widget_->setContentsMargins(3,5,3,0);
                internal_widget_->setMaximumSize(62+6,23+5);
                break;
            }
            case UiDefines::Notifications:
            {
                normal_image += "uibutton_NOTIF_normal.png";
                hover_image += "uibutton_NOTIF_hover.png";
                clicked_image += "uibutton_NOTIF_click.png";
                fixed_size = QSize(52,23);
                internal_widget_->setContentsMargins(3,5,3,0);
                internal_widget_->setMaximumSize(52+6,23+5);
                break;
            }
            case UiDefines::Teleport:
            {
                normal_image += "uibutton_TEL_normal.png";
                hover_image += "uibutton_TEL_hover.png";
                clicked_image += "uibutton_TEL_click.png";
                fixed_size = QSize(62,23);
                internal_widget_->setContentsMargins(3,5,3,0);
                internal_widget_->setMaximumSize(62+6,23+5);
                break;
            }
            default:
                return;
        }
        QString style_normal("QPushButton { background-color: transparent; border: 0px; background-position: top left; background-repeat: no-repeat; background-image: url('" + normal_image + "'); }");
        QString style_hover("QPushButton::hover { background-color: transparent; border: 0px; background-image: url('" + hover_image + "'); }");
        QString style_clicked("QPushButton::pressed { background-color: transparent; border: 0px; background-image: url('" + clicked_image + "'); }");
        actionButton->setStyleSheet(style_normal + style_hover + style_clicked);
        actionButton->setMinimumSize(fixed_size);
        actionButton->setMaximumSize(fixed_size);
    }

    void ControlPanelButton::Clicked()
    {
        emit ControlButtonClicked(type_);
    }
}