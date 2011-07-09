#pragma once

#include <iostream>
#include <QWidget>

class RedirectedPaintWidget : public QWidget
{
public:
    QImage target;

    RedirectedPaintWidget()
    {
    }

    virtual QPaintEngine *paintEngine() const
    {
        return target.paintEngine();
    }

    virtual int devType() const
    {
        return 8;
    }

    void ResizeSurface(QSize newSize)
    {
        target = QImage(newSize, QImage::Format_ARGB32);
        target.fill(0);
        std::cout << "Resized paint target surface to " << newSize.width() << "x" << newSize.height() << std::endl;
    }

    void resizeEvent(QResizeEvent *event)
    {
        ResizeSurface(size());
    }
};
