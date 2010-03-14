// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotifyLabel_h
#define incl_UiModule_NotifyLabel_h

#include <QLabel>

namespace CoreUi
{
    class NotifyLabel : public QLabel
    {

    Q_OBJECT

    public:
        NotifyLabel(const QString &text, int duration_msec);
        virtual ~NotifyLabel();
    
    public slots:
        void TimeOut();

    signals:
        void DestroyMe(CoreUi::NotifyLabel *me);

    };
}

#endif // incl_UiModule_NotifyLabel_h