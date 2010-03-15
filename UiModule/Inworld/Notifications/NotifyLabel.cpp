// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "NotifyLabel.h"



#include "MemoryLeakCheck.h"

namespace CoreUi
{

    NotifyLabel::NotifyLabel(const QString &text, int duration_msec)
        : QGraphicsProxyWidget(),
        internal_widget_(new QWidget()),
        anim_(this, "opacity"),
        current_time_(duration_msec)
    {
        setupUi(internal_widget_);
        setWidget(internal_widget_);
        text_label->setText(text);
        time_board->display(duration_msec);
        anim_.setStartValue(1);
        anim_.setEndValue(0);
        timer_.setInterval(20);
        //fadetime is 10% of the messages whole displaytime
        anim_.setDuration(200);
        connect(&anim_, SIGNAL(finished()), this, SLOT(AnimationFinished())); 
        connect(closebutton, SIGNAL(clicked()),this, SLOT(CloseClicked()) );
        
    }

    void NotifyLabel::CloseClicked()
    {
        hide();
        emit DestroyMe(this);
    }

    void NotifyLabel::ShowNotification()
    {
        QObject::connect(&timer_, SIGNAL(timeout()), this, SLOT(TimeOut()));
        show();
        timer_.start();
    }

    void NotifyLabel::AnimationFinished()
    {
        hide();
        emit DestroyMe(this);
    }

    NotifyLabel::~NotifyLabel()
    {
    }

    void NotifyLabel::TimeOut()
    {
        current_time_ -= timer_.interval();
        if(current_time_>0)
        {
            time_board->display(current_time_);
        }
        else
        {
            timer_.stop();
            anim_.start();
        }
    }
}