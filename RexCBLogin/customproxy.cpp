#include "../Core/DebugOperatorNew.h"
#include "customproxy.h"

#include <QtGui>
#include "../Core/MemoryLeakCheck.h"

CustomProxy::CustomProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
: QGraphicsProxyWidget(parent, wFlags), timeLineShow(0)
{
	setTransform( QTransform().scale(0, 0) );
	timeLineShow = new QTimeLine(1000, 0);
	QObject::connect( timeLineShow, SIGNAL(valueChanged(qreal)), this, SLOT(updateShowStep(qreal)) );
}

CustomProxy::~CustomProxy()
{
    delete timeLineShow;
    timeLineShow = 0;
}

void CustomProxy::showEvent(QShowEvent *sEvent)
{
	timeLineShow->start();
	QGraphicsProxyWidget::showEvent(sEvent);
}

void CustomProxy::updateShowStep(qreal step)
{
	QRectF myRect = boundingRect();
	QTransform t;
	
	t.rotate(step * 360, Qt::YAxis);
	t.scale(step, step);
	t.translate(-myRect.width() / 2, -myRect.height() / 2);
	
	setTransform(t);
}