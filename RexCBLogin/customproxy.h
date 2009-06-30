#ifndef incl_CustomProxy_h
#define incl_CustomProxy_h

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

#endif // incl_CustomProxy_h
