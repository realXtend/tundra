// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ControlPanelButton_h
#define incl_UiModule_ControlPanelButton_h

#include "UiTypes.h"

#include <QGraphicsProxyWidget>
#include "ui_ControlPanelButton.h"

namespace CoreUi
{
    class ControlPanelButton : public QGraphicsProxyWidget, private Ui::ControlPanelButton
    {
    
    Q_OBJECT

    public:
        ControlPanelButton(UiServices::ControlButtonType type);
		QWidget *internal_widget_;

    public slots:
        qreal GetContentWidth();
        UiServices::ControlButtonType GetType();
        void CheckStyle(bool item_visible);
		QWidget* GetInternalWidget() { return internal_widget_; }
		
        
    private slots:
        void InitStyle();
        void Clicked();

    signals:
        void ControlButtonClicked(UiServices::ControlButtonType);

    private:
        
        UiServices::ControlButtonType type_;

    };
}

#endif