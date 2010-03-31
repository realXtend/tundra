// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_PersonalWidget_h
#define incl_UiModule_PersonalWidget_h

#include <QGraphicsProxyWidget>

#include "Inworld/View/UiProxyWidget.h"
#include "ui_PersonalWidget.h"

namespace CoreUi
{
    class PersonalWidget : public QGraphicsProxyWidget, private Ui::PersonalWidget
    {
    
    Q_OBJECT

    public:
        PersonalWidget();

    public slots:
        void SetAvatarWidget(UiServices::UiProxyWidget *avatar_widget);
        void SetInventoryWidget(UiServices::UiProxyWidget *inventory_widget);

    private slots:
        void AvatarToggle();
        void InventoryToggle();

        void AvatarVisibilityChanged(bool visible);
        void InventoryVisibilityChanged(bool visible);
        void CheckStyle(bool pressed_down, QString type);

    private:
        QWidget *internal_widget_;
        UiServices::UiProxyWidget *avatar_widget_;
        UiServices::UiProxyWidget *inventory_widget_;

        bool first_show_avatar_;
        bool first_show_inv_;

    };
}

#endif