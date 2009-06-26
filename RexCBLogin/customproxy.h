#ifndef CUSTOMPROXY_H
#define CUSTOMPROXY_H

#include <QGraphicsProxyWidget>
#include <QTimeLine>

class CustomProxy 
	: public QGraphicsProxyWidget
{
	Q_OBJECT

public:
	CustomProxy(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
	void showEvent(QShowEvent *sEvent);

public slots:
	void updateShowStep(qreal step);

private:
	QTimeLine *timeLineShow;

};

#endif
