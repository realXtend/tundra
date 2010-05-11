#include "StableHeaders.h"
#include "HoveringNameController.h"
#include <QPushButton>
#include <QMouseEvent>
#include <QDebug>
#include "RexLogicModule.h"

namespace RexLogic
{

    HoveringNameController::HoveringNameController()
    :text_padding_(10.0f)
    {
        Ui::HoveringName::setupUi(this);
    }

    HoveringNameController::~HoveringNameController()
    {

    }

    void HoveringNameController::SetText(const QString &text)
    {
        Real l_width = label->fontMetrics().width(text);
        Real l_height = label->fontMetrics().height();
        QFont font = label->font();
        Real scale_W = (this->width()- text_padding_) / l_width;
        Real scale_H = (this->height())/l_height;
        Real scale = scale_W<scale_H? scale_W:scale_H;
        
        if(scale>0)
            font.setPointSizeF(font.pointSizeF()*scale);
        label->setFont(font);
        label->setText(text);
    }


}
