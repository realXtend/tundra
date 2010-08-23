/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LoginWidget.h
 *  @brief  Simple login widget for OpenSim and realXend authentication methods.
 */

#ifndef incl_LoginModule_LoginWidget_h
#define incl_LoginModule_LoginWidget_h

#include "ui_LoginWidget.h"

#include <QTimer>

/// Simple login widget for OpenSim and realXend authentication methods.
class LoginWidget : public QWidget, private Ui::LoginWidget
{
    Q_OBJECT

public:
    LoginWidget(QWidget *parent, const QMap<QString,QString> &stored_login_data);

public slots:
    QMap<QString, QString> GetLoginInfo() const;
    void StatusUpdate(bool connecting, const QString &message);

private slots:
    void InitWidget(const QMap<QString,QString> &stored_login_data);
    void ParseInputAndConnect();
    void UpdateProgressBar();

private:
    QTimer *progress_timer_;
    int progress_direction_;

signals:
    void ConnectOpenSim(const QMap<QString, QString> &login_data);
    void ConnectRealXtend(const QMap<QString, QString> &login_data);
    void ConnectingUiUpdate(const QString &message);
};

#endif
