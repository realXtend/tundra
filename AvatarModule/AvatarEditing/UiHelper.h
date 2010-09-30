// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Avatar_UiHelper_h
#define incl_Avatar_UiHelper_h

#include <QObject>
#include <QList>
#include <QGraphicsProxyWidget>

#include "ui_AvatarToolbar.h"
#include "ui_AvatarInfoWidget.h"

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
            QGraphicsProxyWidget *CreateInfoWidget();
            QGraphicsProxyWidget *CreateProxy(QWidget *widget);

            void ShowStatus(const QString &message, int msec_timeout = 0);
            void ShowError(const QString &error, int msec_timeout = 0);
            void HideInfo();

        signals:
            void ExitRequest();
        
        private:
            QList<QWidget*> cleanup_widgets_;

            Ui::AvatarToolbar toolbar_ui_;
            Ui::AvatarInfoWidget info_ui_;

            QGraphicsProxyWidget *info_proxy_;
        };
    }
}

#endif