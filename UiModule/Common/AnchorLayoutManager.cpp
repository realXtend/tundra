// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AnchorLayoutManager.h"

#include "Inworld/View/UiProxyWidget.h"

#include <QGraphicsScene>
#include <QGraphicsWidget>
#include <QGraphicsAnchorLayout>
#include <QGraphicsLayoutItem>
#include <QGraphicsWidget>

#include "MemoryLeakCheck.h"

namespace CoreUi
{
    AnchorLayoutManager::AnchorLayoutManager(QObject *parent, QGraphicsScene *scene) :
        QObject(parent),
        scene_(scene)
    {
        InitLayout();
        AdjustLayoutContainer(scene_->sceneRect());

        connect(scene_, SIGNAL( sceneRectChanged(const QRectF &) ), SLOT( AdjustLayoutContainer(const QRectF &) ));
    }

    AnchorLayoutManager::~AnchorLayoutManager()
    {

    }

    // Private

    void AnchorLayoutManager::InitLayout()
    {
        anchor_layout_ = new QGraphicsAnchorLayout();
        anchor_layout_->setSpacing(0);
        anchor_layout_->setContentsMargins(0,0,0,0);

        layout_container_ = new QGraphicsWidget();
        layout_container_->setLayout(anchor_layout_);

        scene_->addItem(layout_container_);
    }

    void AnchorLayoutManager::AdjustLayoutContainer(const QRectF &new_rect)
    {
        layout_container_->setGeometry(new_rect);
        foreach (QGraphicsWidget *widget, full_screen_widgets_)
            widget->setGeometry(new_rect);
    }

    void AnchorLayoutManager::CheckVisibility(QGraphicsLayoutItem *layout_item)
    {
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(layout_item);
        if (!widget)
            return;
        if (!widget->isVisible())
            widget->show();
    }

    // Public

    void AnchorLayoutManager::AddCornerAnchor(QGraphicsLayoutItem *layout_item, Qt::Corner layout_anchor_corner, Qt::Corner widget_anchor_corner)
    {
        anchor_layout_->addCornerAnchors(anchor_layout_, layout_anchor_corner, layout_item, widget_anchor_corner);
        CheckVisibility(layout_item);
    }

    void AnchorLayoutManager::AddSideAnchor(QGraphicsLayoutItem *layout_item, Qt::Orientation orientation)
    {
        anchor_layout_->addAnchors(anchor_layout_, layout_item, orientation);
        CheckVisibility(layout_item);
    }

    void AnchorLayoutManager::AddFullscreenWidget(QGraphicsWidget *graphics_widget)
    {
        AddItemToScene(graphics_widget);
        full_screen_widgets_.append(graphics_widget);
    }

    void AnchorLayoutManager::AddItemToScene(QGraphicsWidget *graphics_widget)
    {
        scene_->addItem(graphics_widget);
    }

    void AnchorLayoutManager::RemoveItemFromScene(QGraphicsWidget *graphics_widget)
    {
        scene_->removeItem(graphics_widget);
    }
}