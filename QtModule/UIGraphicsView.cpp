#include "StableHeaders.h"

#include "UIGraphicsView.h"

namespace QtUI
{

UIGraphicsView::UIGraphicsView(QWidget* parent) : QGraphicsView(parent)
{}

UIGraphicsView::~UIGraphicsView()
{}

void UIGraphicsView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    emit ViewResized(event);
}

}