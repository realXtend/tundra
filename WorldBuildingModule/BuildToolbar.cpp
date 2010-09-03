// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "BuildToolbar.h"
#include "UiProxyWidget.h"

#include <QPushButton>
#include <QHBoxLayout>

namespace WorldBuilding
{
    namespace Ui
    {
        BuildToolbar::BuildToolbar() :
            QGraphicsProxyWidget(),
            widget_(new QWidget())
        {
            setupUi(widget_);
            setWidget(widget_);
            slider_lights->hide();
            content_widget->setLayout(new QHBoxLayout());
            content_widget->layout()->setContentsMargins(6,5,9,3);
        }

        void BuildToolbar::AddButton(const QString &name, QGraphicsProxyWidget *widget)
        {
            UiProxyWidget *naali_proxy = dynamic_cast<UiProxyWidget*>(widget);
            if (!naali_proxy)
                return;

            QPushButton *button = new QPushButton(name);
            connect(button, SIGNAL(clicked()), naali_proxy, SLOT(ToggleVisibility()));
            content_widget->layout()->addWidget(button);
        }

        void BuildToolbar::RemoveAllButtons()
        {
            QLayout *layout = content_widget->layout();
            QLayoutItem *item = 0;
            while ((item =layout->takeAt(0)) != 0)
                delete item;
        }
    }
}
