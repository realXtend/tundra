// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "HoveringNameController.h"

#include <QPushButton>
#include <QMouseEvent>

HoveringNameController::HoveringNameController() :
    text_padding_(30.0f)
{
    Ui::HoveringName::setupUi(this);
}

HoveringNameController::~HoveringNameController()
{
}

void HoveringNameController::SetText(const QString &text)
{
    label->setText(text);
}
