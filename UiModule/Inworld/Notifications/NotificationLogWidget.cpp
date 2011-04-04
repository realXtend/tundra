//$ HEADER_MOD_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Inworld/Notifications/NotificationLogWidget.h"

#include <QDateTime>
#include <QIcon>

namespace CoreUi
{
    NotificationLogWidget::NotificationLogWidget(bool active, QWidget *content_widget, QDateTime timestamp, QString result_title, QString result, QIcon icon) :
        QWidget(),
        content_widget_(content_widget)
    {
        setupUi(this);

        // Init Ui
		//$ BEGIN_MOD $
		timestampLabel->setText(timestamp.time().toString("hh:mm:ss"));
        timestampLabel->setText(timestamp.time().toString("hh:mm:ss") + "  " +result_title);
		//$ END_MOD $
        resultLabel->setText(result);
        resultTitleLabel->setText(result_title);
        mainLayout->insertWidget(1, content_widget_);

        // Set icon
        if (icon.isNull())
        {
            QPixmap default_pixmap("./data/ui/images/notifications/default_icon.png");
            iconLabel->setPixmap(default_pixmap);
        }
        else
            iconLabel->setPixmap(icon.pixmap(iconLabel->size()));

        // Hide elements if still active or there is nothing to show
        if (active || result_title.isEmpty() || result.isEmpty())
        {
            resultLabel->hide();
            resultTitleLabel->hide();
            mainLayout->update();
        }


    }

    void NotificationLogWidget::Deactivate(QString title, QString result)
    {
        // Show result elements if there is content
        if (result.isEmpty() || title.isEmpty())
        {
            resultLabel->hide();
            resultTitleLabel->hide();
        }
        else
        {
            resultLabel->setText(result);
            resultTitleLabel->setText(title);
            resultLabel->show();
            resultTitleLabel->show();
            mainLayout->update();
        }

    }
}