/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiWidget.h
 *  @brief  Represents external UI widget created by embedding QWidget.
 */

#ifndef incl_Ui_UiWidget_h
#define incl_Ui_UiWidget_h

#include <qwidget.h>

class UiWidget :  public QWidget
{
     Q_OBJECT

public:
    /// Constructor.
    /** @param  widget The widget which will be embedded to the ui widget.
        @param  flags Window flags. 
        @param  params list of string parameters.
    */
    UiWidget(QWidget *widget, QWidget *parent = 0, Qt::WindowFlags flags = 0, const QStringList &params = QStringList());
    
    /// Destructor.
    ~UiWidget(void);

public slots:
    virtual void setVisible(bool visible);

    /// Toggles visibility
    void toggleVisibility();

    /// Sends visibilityChanged signal
    /** @param visible visitility to signal
    */
    void sendVisibilityChanged(bool visible);

    /// Returns contained widget
    QWidget* widget() {return widget_;}

signals:
    ///Emitted when visibility changes.
    /**@param visible visibility
    */
    void visibilityChanged(bool visible);

private:
    QWidget* widget_;
};
#endif
