/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   LoginWidget.h
 *  @brief  Simple full screen login widget for OpenSim and realXend authentication methods.
 */

#ifndef incl_LoginModule_LoginWidget_h
#define incl_LoginModule_LoginWidget_h

#include "ui_LoginWidget.h"

#include <QTimer>

/// Simple full screen login widget for OpenSim and realXend authentication methods.
class LoginWidget : public QWidget, private Ui::LoginWidget
{
    Q_OBJECT

public:
    /** Constructor.
     *  @param loginInfo
     */
    explicit LoginWidget(const QMap<QString,QString> &login_data);

public slots:
    QMap<QString, QString> GetLoginInfo() const;
//    void StatusUpdate(bool connecting, const QString &message);
    void SetStatus(const QString &message);

    ///
    void Connected();

signals:
    ///
    void Connect(const QMap<QString, QString> &login_data);

    ///
    void ExitClicked();

private slots:
    ///
    void ParseInputAndConnect();

    ///
    void StartProgressBar();

    ///
    void UpdateProgressBar();

    ///
    void StopProgressBar();

    ///
    void Exit();

private:
    ///
    QTimer *progress_timer_;

    ///
    int progress_direction_;
};

#endif
