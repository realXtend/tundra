// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_SettingsWidget_h
#define incl_UiModule_SettingsWidget_h

#include "ui_SettingsWidget.h"

#include <QGraphicsProxyWidget>
#include <QShowEvent>

class QPropertyAnimation;
class QGraphicsScene;

namespace CoreUi
{
    class ControlPanelManager;

    class SettingsWidget : public QGraphicsProxyWidget, private Ui::SettingsWidget
    {
        
    Q_OBJECT

    public:
        SettingsWidget(QGraphicsScene *scene, ControlPanelManager *conrol_panel_manager);

    public slots:
        void AddWidget(QWidget *widget, const QString &tab_name) const;
        void AnimatedHide();

    protected:
        void showEvent(QShowEvent *show_event);

    private slots:
        void SaveSettings();
        void Canceled();
        void AnimationsFinished();

        void SceneRectChanged(const QRectF &scene_rect);
        void OpacitySliderChanged(int new_value);

    private:
        QWidget *internal_widget_;
        QPropertyAnimation *visibility_animation_;

        ControlPanelManager *panel_;

    signals:
        void SaveSettingsClicked();
        void CancelClicked();
        void Hidden();
        void NewUserInterfaceSettingsApplied(int new_opacity, int new_animation_speed);

    };
}

#endif // incl_UiModule_SettingsWidget_h
