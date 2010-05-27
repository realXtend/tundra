#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MultiEditWidget.h"
#include "ECEditorModule.h"

#include <QInputDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QPushButton>
#include <QLayout>
#include <QInputDialog>

#include <QUiLoader>
#include <QFile>

#include "MemoryLeakCheck.h"

namespace ECEditor
{
    void MultiSelectButton::mousePressEvent(QMouseEvent *event)
    {
        if(event->button() == Qt::MouseButton::LeftButton)
        {
            emit ButtonClicked(); 
        }
        QPushButton::mousePressEvent(event);
    }

    MultiEditWidget::MultiEditWidget(QWidget *parent):
        QWidget(parent)
    {
        QHBoxLayout *layout = new QHBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
        
        text_label_ = new QLabel();
        layout->addWidget(text_label_);
        text_label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        text_label_->setFocusPolicy(Qt::NoFocus);

        button_ = new MultiSelectButton();
        button_->setText("...");
        layout->addWidget(button_);
        button_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        button_->resize(20, 30);

        QSpacerItem *spacer = new QSpacerItem(50, 20, QSizePolicy::Maximum, QSizePolicy::Maximum);
        layout->insertSpacerItem(1, spacer);
        QObject::connect(button_, SIGNAL(ButtonClicked()), this, SIGNAL(ButtonClicked()));
    }

    MultiEditWidget::~MultiEditWidget()
    {

    }

    void MultiEditWidget::SetLabelText(const QString &text)
    {
        text_label_->setText(text);
    }
}