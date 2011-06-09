/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiWidget.h
 *  @brief  Represents external UI widget created by embedding QWidget.
 */

#ifndef incl_Ui_UiWidget_h
#define incl_Ui_UiWidget_h

#include <QWidget.h>

class UiWidget :  public QWidget
{
     Q_OBJECT

public:
    UiWidget(QWidget *widget, QWidget *parent = 0, Qt::WindowFlags flags = 0, const QStringList &params = QStringList());
    ~UiWidget(void);

public slots:
    virtual void setVisible(bool visible);
    void toogleVisibility();
    void sendVisibilityChanged(bool visible);
    QWidget* widget() {return widget_;}

signals:
    void visibilityChanged(bool visible);

private:
    QWidget* widget_;
};
#endif
