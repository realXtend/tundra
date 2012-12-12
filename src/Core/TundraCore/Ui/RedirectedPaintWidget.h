// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"

#include <iostream>
#include <QWidget>

/// @cond PRIVATE
class TUNDRACORE_API RedirectedPaintWidget : public QWidget
{
    Q_OBJECT

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

    void resizeEvent(QResizeEvent * /*event*/)
    {
        ResizeSurface(size());
    }
};
/// @endcond
