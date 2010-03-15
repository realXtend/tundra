// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_TraditionalLoginWidget_h
#define incl_UiModule_TraditionalLoginWidget_h

#include "ui_TraditionalLoginWidget.h"

#include <QTimer>

namespace CoreUi
{
    namespace Classical
    {
        class TraditionalLoginWidget : public QWidget, private Ui::TraditionalLoginWidget
        {
            
        Q_OBJECT

        public:
            TraditionalLoginWidget(QWidget *parent, QMap<QString,QString> stored_login_data);

        public slots:
            void RemoveEtherButton();
            QMap<QString, QString> GetLoginInfo();
            void StatusUpdate(bool connecting, QString message);

        private slots:
            void InitWidget(QMap<QString,QString> stored_login_data);
            void ParseInputAndConnect();
            void UpdateProgressBar();

        private:
            QTimer *progress_timer_;
            int progress_direction_;

        signals:
            void ConnectOpenSim(QMap<QString, QString>);
            void ConnectRealXtend(QMap<QString, QString>);
            void ConnectingUiUpdate(QString message);
        };
    }
}

#endif // incl_UiModule_TraditionalLoginWidget_h
