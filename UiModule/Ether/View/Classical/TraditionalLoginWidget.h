// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_TraditionalLoginWidget_h
#define incl_UiModule_TraditionalLoginWidget_h

#include "ui_TraditionalLoginWidget.h"

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
            QMap<QString, QString> GetLoginInfo();

        private:
            void InitWidget(QMap<QString,QString> stored_login_data);

        private slots:
            void ParseInputAndConnect();

        signals:
            void ConnectOpenSim(QMap<QString, QString>);
            void ConnectRealXtend(QMap<QString, QString>);
        };
    }
}

#endif // incl_UiModule_TraditionalLoginWidget_h
