// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NotifyLabel.h"

namespace CoreUi
{

    NotifyLabel::NotifyLabel(const QString &text)
        : QLabel(text),
          hide_timeline_(new QTimeLine(5000, this))
    {
        connect(hide_timeline_, SIGNAL(valueChanged(qreal)), SLOT(UpdateOpacity(qreal)));
        //hide_timeline_->start();
    }

    NotifyLabel::~NotifyLabel()
    {

    }

    void NotifyLabel::UpdateOpacity(qreal step)
    {
        setWindowOpacity(1.0-step);
    }
}