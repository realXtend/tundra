// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AnchorLayoutManager.h"

#include "UiProxyWidget.h"

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
        //Get here from parent InworldScene controller access to signal whem centralWidget of QMainWindow changes. Or at least change Scene when QMainWidget changes too!!

        connect(scene_, SIGNAL( sceneRectChanged(const QRectF &) ), SLOT( AdjustLayoutContainer(const QRectF &) ));
    }

    AnchorLayoutManager::~AnchorLayoutManager()
    {

    }

    // Private

    void AnchorLayoutManager::InitLayout()
    {
        anchor_layout_ = new QGraphicsAnchorLayout();
        anchor_layout_->setHorizontalSpacing(2);
        anchor_layout_->setVerticalSpacing(10);
        anchor_layout_->setContentsMargins(0,0,0,0);

        layout_container_ = new QGraphicsWidget();
        layout_container_->setLayout(anchor_layout_);

        //scene_->addItem(layout_container_);
        scene_->addItem(layout_container_);
    }

    void AnchorLayoutManager::AdjustLayoutContainer(const QRectF &new_rect)
    {
        layout_container_->setGeometry(new_rect);
        foreach (QGraphicsWidget *widget, full_screen_widgets_)
            widget->setGeometry(new_rect);
    }

    void AnchorLayoutManager::AdjustLayoutContainer(int newWidth, int newHeight)
    {
        layout_container_->resize(newWidth, newHeight);
    }

    void AnchorLayoutManager::CheckVisibility(QGraphicsLayoutItem *layout_item)
    {
        /*QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(layout_item);
        if (!widget)
            return;
        if (!widget->isVisible())
            widget->show();
            */
    }

    // Public
	void AnchorLayoutManager::resetLayout()
	{
		scene_->removeItem(layout_container_);
		InitLayout();
        AdjustLayoutContainer(scene_->sceneRect());
        connect(scene_, SIGNAL( sceneRectChanged(const QRectF &) ), SLOT( AdjustLayoutContainer(const QRectF &) ));

	}

    QGraphicsScene *AnchorLayoutManager::GetScene()
    {
        return scene_;
    }

    void AnchorLayoutManager::AddCornerAnchor(QGraphicsLayoutItem *layout_item, Qt::Corner layout_anchor_corner, Qt::Corner widget_anchor_corner)
    {
        anchor_layout_->addCornerAnchors(anchor_layout_, layout_anchor_corner, layout_item, widget_anchor_corner);
    }

    void AnchorLayoutManager::AddSideAnchor(QGraphicsLayoutItem *layout_item, Qt::Orientation orientation)
    {
        anchor_layout_->addAnchors(anchor_layout_, layout_item, orientation);
    }

    void AnchorLayoutManager::AnchorWidgetsHorizontally(QGraphicsLayoutItem *first_item, QGraphicsLayoutItem *second_item, bool align_top)
    {
        anchor_layout_->addAnchor(first_item, Qt::AnchorLeft, second_item, Qt::AnchorRight);
        if (align_top)
            anchor_layout_->addAnchor(first_item, Qt::AnchorTop, second_item, Qt::AnchorTop);
        else
            anchor_layout_->addAnchor(first_item, Qt::AnchorBottom, second_item, Qt::AnchorBottom);
    }

	void AnchorLayoutManager::AnchorWidgetsVertically(QGraphicsLayoutItem *first_item, QGraphicsLayoutItem *second_item, bool align_left)
    {
		anchor_layout_->addAnchor(first_item, Qt::AnchorTop, second_item, Qt::AnchorBottom);
        if (align_left)
            anchor_layout_->addAnchor(first_item, Qt::AnchorLeft, second_item, Qt::AnchorLeft);
        else
            anchor_layout_->addAnchor(first_item, Qt::AnchorRight, second_item, Qt::AnchorRight);
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