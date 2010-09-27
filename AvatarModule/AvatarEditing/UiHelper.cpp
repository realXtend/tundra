// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "UiHelper.h"

#include "ui_AvatarToolbar.h"

namespace Avatar
{
    namespace Helpers
    {
        UiHelper::UiHelper(QObject *parent) :
            QObject(parent)
        {
        }

        UiHelper::~UiHelper()
        {
            foreach(QWidget *widget, cleanup_widgets_)
                SAFE_DELETE(widget);
        }

        QGraphicsProxyWidget *UiHelper::CreateToolbar()
        {
            QWidget *toolbar = new QWidget();
            Ui::AvatarToolbar *toolbar_ui = new Ui::AvatarToolbar();
            toolbar_ui->setupUi(toolbar);

            connect(toolbar_ui->button_exit, SIGNAL(clicked()), SIGNAL(ExitRequest()));
            
            cleanup_widgets_ << toolbar;
            QGraphicsProxyWidget *toolbar_proxy = new QGraphicsProxyWidget(0, Qt::Widget);
            toolbar_proxy->setWidget(toolbar);
            return toolbar_proxy;
        }
    }
}