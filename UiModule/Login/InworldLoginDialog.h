// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_InworldLoginDialog_h
#define incl_UiModule_InworldLoginDialog_h

//#include "Foundation.h"

#include <QObject>
#include <QRadioButton>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>

namespace Foundation
{
    class Framework;
}

namespace CoreUi
{
    class InworldLoginDialog : public QObject
    {
        Q_OBJECT

    public:
        explicit InworldLoginDialog(Foundation::Framework *framework);
        virtual ~InworldLoginDialog();

    private:
        void SetConfigsToUi();

        QDialog *inworld_login_widget_;
        
        QRadioButton *radiobutton_opensim_;
        QRadioButton *radiobutton_realxtend_;

        QLineEdit *line_edit_username_;
        QLineEdit *line_edit_password_;

        QLabel *status_message_label_;

        QString current_server_address_;
        QString opensim_username_;
        QString realxtend_username_;
        QString realxtend_authserver_;

        Foundation::Framework *framework_;

    public slots:
        void ShowModalDialog(const QString &server_address);
    
    private slots:
        void ToggleLoginShowMode();
        void ParseAndTryToConnect();

    signals:
        void TryLogin(QMap<QString,QString> &);

    };
}

#endif // incl_UiModule_InworldLoginDialog_h