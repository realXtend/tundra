/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   UiProxyWidget.h
 *  @brief  Represents Naali UI widget created by embedding QWidget to the same
 *          canvas as the in-world 3D scene.
 */

#ifndef incl_Ui_UiWidget_h
#define incl_Ui_UiWidget_h

#include <QWidget.h>

class UiWidget :  public QWidget
{
     Q_OBJECT

public:
    UiWidget(QWidget *widget, Qt::WindowFlags flags);
    ~UiWidget(void);
};
#endif
