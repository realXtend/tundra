// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_PersonalWidget_h
#define incl_UiModule_PersonalWidget_h

#include <QGraphicsProxyWidget>

#include "UiTypes.h"
#include "UiProxyWidget.h"

#ifdef PLAYER_VIEWER
#include "ui_PersonalWidgetPlayer.h"
#else
#include "ui_PersonalWidgetPlayer.h"
#endif

namespace CoreUi
{
    class PersonalWidget : public QGraphicsProxyWidget, private Ui::PersonalWidget
    {
    
    Q_OBJECT

    public:
        PersonalWidget();

    public slots:
        void SetAvatarWidget(UiProxyWidget *avatar_widget);

    private slots:
        void AvatarToggle();

        void AvatarVisibilityChanged(bool visible);
        void CheckStyle(bool pressed_down, QString type);

    private:
        QWidget *internal_widget_;
        UiProxyWidget *avatar_widget_;

        bool first_show_avatar_;

signals:
        void ControlButtonClicked(UiServices::ControlButtonType);

    };
}

#endif