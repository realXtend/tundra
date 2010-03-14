// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "NotifyLabel.h"

#include <QTimer>

#include "MemoryLeakCheck.h"

namespace CoreUi
{

    NotifyLabel::NotifyLabel(const QString &text, int duration_msec)
        : QLabel(text)
    {
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        QTimer::singleShot(duration_msec, this, SLOT(TimeOut()));
    }

    NotifyLabel::~NotifyLabel()
    {
    }

    void NotifyLabel::TimeOut()
    {
        emit DestroyMe(this);
    }
}