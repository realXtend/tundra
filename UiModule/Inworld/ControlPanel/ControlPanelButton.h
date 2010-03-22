// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlPanelButton_h
#define incl_UiModule_ControlPanelButton_h

#include "UiDefines.h"

#include <QGraphicsProxyWidget>
#include "ui_ControlPanelButton.h"

namespace CoreUi
{
    class ControlPanelButton : public QGraphicsProxyWidget, private Ui::ControlPanelButton
    {
    
    Q_OBJECT

    public:
        ControlPanelButton(UiDefines::ControlButtonType type);

    public slots:
        qreal GetContentWidth();
        UiDefines::ControlButtonType GetType();
        void CheckStyle(bool item_visible);
        
    private slots:
        void InitStyle();
        void Clicked();

    signals:
        void ControlButtonClicked(UiDefines::ControlButtonType);

    private:
        QWidget *internal_widget_;
        UiDefines::ControlButtonType type_;

    };
}

#endif