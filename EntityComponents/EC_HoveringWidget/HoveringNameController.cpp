// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HoveringNameController.h"

#include <QPushButton>
#include <QMouseEvent>
#include <QPixmap>

HoveringNameController::HoveringNameController() :
    text_padding_(30.0f),
    pixmap_(0)
{
    Ui::hoveringName::setupUi(this);
}

HoveringNameController::~HoveringNameController()
{
}

void HoveringNameController::SetText(const QString &text)
{
    text_ = text;
    textLabel->setText(text);
}

QString HoveringNameController::GetText() const
{
    return text_;
}

void HoveringNameController::SetPixmap(QPixmap* pixmap)
{
    pixmap_ = pixmap;
    if (pixmap_)
        pixmapLabel->setPixmap(*pixmap_);
}

QPixmap* HoveringNameController::GetPixmap()
{
    return pixmap_;
}
