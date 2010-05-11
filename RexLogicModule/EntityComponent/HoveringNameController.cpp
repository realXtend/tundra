#include "StableHeaders.h"
#include "HoveringNameController.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
#include "RexLogicModule.h"

namespace RexLogic
{

    HoveringNameController::HoveringNameController()
    :text_padding_(20.0f)
    {
        Ui::HoveringName::setupUi(this);
    }

    HoveringNameController::~HoveringNameController()
    {

    }

    void HoveringNameController::SetText(const QString &text)
    {
        Real l_width = label->fontMetrics().width(text);
        QFont font = label->font();
        Real scale = (this->width()- text_padding_) / l_width;
        if(scale>0)
            font.setPointSizeF(font.pointSizeF()*scale);
        label->setFont(font);
        label->setText(text);
    }


}
