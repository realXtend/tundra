// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_TeleportWidget_h
#define incl_UiModule_TeleportWidget_h

#include "ui_TeleportWidget.h"
#include "RexUUID.h"

#include <QGraphicsProxyWidget>
#include <QShowEvent>

class QPropertyAnimation;
class QGraphicsScene;

namespace CoreUi
{
    class ControlPanelManager;

    struct MapBlock
    {
        RexUUID agentID;
        uint32_t flags;
        uint16_t regionX;
        uint16_t regionY;
        std::string regionName;
        uint8_t access;
        uint32_t regionFlags;
        uint8_t waterHeight;
        uint8_t agents;
        RexUUID mapImageID;
    };

    class TeleportWidget : public QGraphicsProxyWidget, private Ui::TeleportWidget
    {
        
    Q_OBJECT

    public:        
        TeleportWidget(QGraphicsScene *scene, ControlPanelManager *conrol_panel_manager);
        void SetMapBlocks(const QList<CoreUi::MapBlock> &map_blocks);

    public slots:
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

    signals:
        void CancelClicked();
        void Hidden();
        void StartTeleport(QString start_location);

    };
}

#endif // incl_UiModule_TeleportWidget_h
