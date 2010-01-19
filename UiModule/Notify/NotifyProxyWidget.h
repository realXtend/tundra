// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_NotifyProxyWidget_h
#define incl_UiModule_NotifyProxyWidget_h

#include <QRectF>
#include <QGraphicsProxyWidget>

namespace CoreUi
{
    class NotifyProxyWidget : public QGraphicsProxyWidget
    {
    
    Q_OBJECT
    
    public:
        NotifyProxyWidget(QWidget *widget);
        virtual ~NotifyProxyWidget();

    public slots:
        void CheckPosition();

    private slots:
        void SceneRectChanged(const QRectF &new_scene_rect);

    protected:
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

    };
}

#endif // incl_UiModule_NotifyProxyWidget_h