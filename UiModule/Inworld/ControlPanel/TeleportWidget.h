// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_TeleportWidget_h
#define incl_UiModule_TeleportWidget_h

#include "NetworkEvents.h"
#include "UiModuleApi.h"
#include "ui_TeleportWidget.h"

#include <QGraphicsProxyWidget>
#include <QShowEvent>

class QPropertyAnimation;
class QGraphicsScene;

namespace CoreUi
{
    class ControlPanelManager;

    class TeleportWidget : public QGraphicsProxyWidget, private Ui::TeleportWidget
    {
        
    Q_OBJECT

    public:        
        TeleportWidget(QGraphicsScene *scene, ControlPanelManager *conrol_panel_manager);

    public slots:
        UI_MODULE_API void SetMapBlocks(const QList<ProtocolUtilities::MapBlock> &map_blocks);
        UI_MODULE_API void SetCurrentRegion(const QString &region_name);
        void AnimatedHide();

    protected:
        void showEvent(QShowEvent *show_event);       

    private slots:
        void Teleport();
        void Cancel();
        void AnimationsFinished();
        void SceneRectChanged(const QRectF &scene_rect);

    private:
        QWidget *internal_widget_;
        ControlPanelManager *panel_;
        std::vector<std::string> *region_names_;

        QPropertyAnimation *visibility_animation_;

        QString current_region_;

    signals:
        void CancelClicked();
        void Hidden();
        void StartTeleport(QString start_location);

    };
}

#endif // incl_UiModule_TeleportWidget_h
