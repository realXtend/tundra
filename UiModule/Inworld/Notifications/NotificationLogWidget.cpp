// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NotificationLogWidget.h"

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
        timestampLabel->setText(timestamp.time().toString("hh:mm:ss"));
        resultLabel->setText(result);
        resultTitleLabel->setText(result_title);
        mainLayout->insertWidget(1, content_widget_);

        // Set icon
        if (icon.isNull())
        {
            QPixmap debug_pixmap(iconLabel->size());
            debug_pixmap.fill(Qt::blue);
            iconLabel->setPixmap(debug_pixmap);
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