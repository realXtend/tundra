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

void HoveringNameController::SetTextHeight(int height)
{
    this->setFixedHeight(height);
    middleFrame->setStyleSheet("background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:1, stop:0.204545 rgba(0, 0, 0, 200), "
        "stop:1 rgba(75, 75, 75, 200)); border: 1px solid grey; border-radius:" + QString::number(height/4) + "px;");
}

void HoveringNameController::SetFontSize(int pt_size)
{
    textLabel->setStyleSheet("background-color: transparent; border: 0px; color: white;	font-size: " + QString::number(pt_size) + 
        "pt; padding-left: " + QString::number(pt_size/3)+ "px; padding-right: " + QString::number(pt_size/3) + "px;");
}

void HoveringNameController::SetPixmap(QPixmap* pixmap)
{
    pixmap_ = pixmap;
    if (pixmap_)
    {
        pixmapLabel->setPixmap(*pixmap_);
        pixmapLabel->resize(pixmap_->size());
    }
}

QPixmap* HoveringNameController::GetPixmap()
{
    return pixmap_;
}
