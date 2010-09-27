// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_UiHelper_h
#define incl_Avatar_UiHelper_h

#include <QObject>
#include <QList>
#include <QGraphicsProxyWidget>

namespace Avatar
{
    namespace Helpers
    {
        class UiHelper : public QObject
        {
            
        Q_OBJECT

        public:
            UiHelper(QObject *parent = 0);
            virtual ~UiHelper();

        public slots:
            QGraphicsProxyWidget *CreateToolbar();

        signals:
            void ExitRequest();
        
        private:
            QList<QWidget*> cleanup_widgets_;
        };
    }
}

#endif