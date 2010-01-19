// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotifyLabel_h
#define incl_UiModule_NotifyLabel_h

#include <QLabel>
#include <QTimeLine>

namespace CoreUi
{
    class NotifyLabel : public QLabel
    {

    Q_OBJECT

    public:
        NotifyLabel(const QString &text);
        virtual ~NotifyLabel();

    private slots:
        void UpdateOpacity(qreal step);

    signals:
        void DestroyMe(CoreUi::NotifyLabel *me);

    private:
        QTimeLine *hide_timeline_;

    };
}

#endif // incl_UiModule_NotifyLabel_h