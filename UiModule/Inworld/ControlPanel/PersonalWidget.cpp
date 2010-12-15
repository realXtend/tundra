//$ HEADER_MOD_FILE $ 
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PersonalWidget.h"

#include <QGraphicsScene>

namespace CoreUi
{
    PersonalWidget::PersonalWidget() :
        QGraphicsProxyWidget(0, Qt::Widget),
        internal_widget_(new QWidget()),
        avatar_widget_(0),
        first_show_avatar_(true)
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);

        connect(avatarToggle, SIGNAL(clicked()), SLOT(AvatarToggle()));

        CheckStyle(false, "avatarToggle");
    }

    void PersonalWidget::SetAvatarWidget(UiProxyWidget *avatar_widget)
    {
    }

    void PersonalWidget::AvatarToggle()
    {
        emit ControlButtonClicked(UiServices::Avatar);

        /*
        if (!avatar_widget_)
            return;

        if (first_show_avatar_)
        {
            qreal padding = 10;
            QPointF avatar_pos(scene()->sceneRect().width() - avatar_widget_->size().width() - padding,
                               scenePos().y() - avatar_widget_->size().height() - padding);
            avatar_widget_->setPos(avatar_pos);
            first_show_avatar_ = false;
        }

        if (avatar_widget_->isVisible())
            avatar_widget_->AnimatedHide();
        else
            avatar_widget_->show();
        */
    }


    void PersonalWidget::AvatarVisibilityChanged(bool visible)
    {
        CheckStyle(visible, "avatarToggle");
    }

    void PersonalWidget::CheckStyle(bool pressed_down, QString type)
    {
        QString base_url = "./data/ui/images/menus/";
        QString image_normal = base_url;
        QString image_hover = base_url;
        QString image_pressed = base_url;
        QPushButton *button;

        if (type == "avatarToggle")
        {
            button = avatarToggle;
            if (!pressed_down)
                image_normal += "uibutton_AVAED_normal.png";
            else
                image_normal += "uibutton_AVAED_click.png";
            image_hover += "uibutton_AVAED_hover.png"; 
            image_pressed += "uibutton_AVAED_click.png";
        }
        else
            return;

        QString style = "QPushButton#" + type + " { background-color: transparent;"
                        "border: 0px; background-image: url('" + image_normal + "'); }"
                        "QPushButton#" + type + "::hover { border: 0px;"
                        "background-image: url('" + image_hover + "'); }"
                        "QPushButton#" + type + "::pressed { border: 0px;"
                        "background-image: url('" + image_pressed + "'); }";

        button->setStyleSheet(style);
    }
}
