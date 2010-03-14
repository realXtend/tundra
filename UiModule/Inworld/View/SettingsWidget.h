// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_SettingsWidget_h
#define incl_UiModule_SettingsWidget_h

#include "ui_SettingsWidget.h"

#include <QWidget>

namespace CoreUi
{
    class SettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        SettingsWidget();
        virtual ~SettingsWidget();

    public slots:
        void AddWidget(QWidget *widget, const QString &tab_name);

    private slots:
        void OpacitySliderChanged(int new_value);
        void ApplySettings();

    private:
        Ui::SettingsWidget settings_ui_;

    signals:
        void NewUserInterfaceSettingsApplied(int new_opacity, int new_animation_speed);
    };
}

#endif // incl_UiModule_SettingsWidget_h
