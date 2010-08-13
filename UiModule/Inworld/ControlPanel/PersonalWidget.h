// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_PersonalWidget_h
#define incl_UiModule_PersonalWidget_h

#include <QGraphicsProxyWidget>

#include "UiProxyWidget.h"
#include "ui_PersonalWidget.h"

namespace CoreUi
{
    class PersonalWidget : public QGraphicsProxyWidget, private Ui::PersonalWidget
    {
    
    Q_OBJECT

    public:
        PersonalWidget();

    public slots:
        void SetAvatarWidget(UiProxyWidget *avatar_widget);
        void SetInventoryWidget(UiProxyWidget *inventory_widget);

    private slots:
        void AvatarToggle();
        void InventoryToggle();

        void AvatarVisibilityChanged(bool visible);
        void InventoryVisibilityChanged(bool visible);
        void CheckStyle(bool pressed_down, QString type);

    private:
        QWidget *internal_widget_;
        UiProxyWidget *avatar_widget_;
        UiProxyWidget *inventory_widget_;

        bool first_show_avatar_;
        bool first_show_inv_;

    };
}

#endif