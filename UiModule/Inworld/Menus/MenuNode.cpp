// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "MenuNode.h"

#include <QPainter>
#include <QGraphicsDropShadowEffect>
#include <QDebug>

namespace CoreUi
{
    MenuNode::MenuNode(const QString& node_name, QUuid id):
            QGraphicsProxyWidget(0),
            node_name_(node_name),
            widget_(new QWidget),
            original_pos_(pos()),
            expanded_pos_(pos()),
            shrunken_pos_(pos())
    {
        setupUi(widget_);
        setWidget(widget_);

        if (id.isNull())
            id_ = QUuid::createUuid();
        else
            id_ = id;

        QFontMetrics metrics(actionPushButton->font());
        expanded_width_ = metrics.width(node_name_);
        actionPushButton->setMinimumSize(35,35);
        actionPushButton->setMaximumSize(50+expanded_width_,35);
        
        base_stylesheet_ = "background-image: url('./data/ui/images/menus/node_bg.png'); background-position: top left; background-repeat: no-repeat;";
        actionPushButton->setStyleSheet(base_stylesheet_);

        QGraphicsDropShadowEffect *shadow_effect = new QGraphicsDropShadowEffect(this);
        shadow_effect->setColor(Qt::white);
        shadow_effect->setOffset(0,0);
        shadow_effect->setBlurRadius(10);
        setGraphicsEffect(shadow_effect);
    }

    void MenuNode::EnableText()
    {
        actionPushButton->setText(node_name_);
    }

    void MenuNode::DisableText()
    {
        actionPushButton->setText("");
    }

    QPushButton *MenuNode::GetMenuButton()
    {
        return actionPushButton;
    }

    QUuid MenuNode::GetID()
    {
        return id_;
    }
}