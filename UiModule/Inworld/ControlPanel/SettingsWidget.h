//$ HEADER_MOD FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_SettingsWidget_h
#define incl_UiModule_SettingsWidget_h

#include "ui_SettingsWidget.h"

#include <QGraphicsProxyWidget>
#include <QShowEvent>

class QPropertyAnimation;
class QGraphicsScene;

namespace UiServices
{
    class UiModule;
}

namespace CoreUi
{
    class ControlPanelManager;

    class SettingsWidget : public QWidget, public Ui::SettingsWidget
    {
        
    Q_OBJECT

    public:
        SettingsWidget(QGraphicsScene *scene, UiServices::UiModule *ui_module);

    public slots:
        void AddWidget(QWidget *widget, const QString &tab_name) const;
        void ToggleVisibility();


    private slots:
        void SaveSettings();
        void Canceled();

        void OpacitySliderChanged(int new_value);

    private:
        QPropertyAnimation *visibility_animation_;
        UiServices::UiModule *ui_module_;

    signals:
        void SaveSettingsClicked();
        void CancelClicked();
        void NewUserInterfaceSettingsApplied(int new_opacity, int new_animation_speed);

    };
}

#endif // incl_UiModule_SettingsWidget_h
