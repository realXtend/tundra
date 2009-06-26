#include "customproxy.h"

#include <QtGui>

CustomProxy::CustomProxy(QGraphicsItem *parent, Qt::WindowFlags wFlags)
	: QGraphicsProxyWidget(parent, wFlags)
{
	setTransform( QTransform().scale(0, 0) );
	timeLineShow = new QTimeLine(1000, this);
	QObject::connect( timeLineShow, SIGNAL(valueChanged(qreal)), this, SLOT(updateShowStep(qreal)) );
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