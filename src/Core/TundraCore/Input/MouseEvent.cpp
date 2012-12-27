// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MouseEvent.h"

#include "MemoryLeakCheck.h"

static int QtMouseButtonEnumToIndex(int mouseButton)
{
    switch(mouseButton)
    {
    case Qt::LeftButton:  return 0;
    case Qt::RightButton: return 1;
    case Qt::MidButton:   return 2;
    case Qt::XButton1:    return 3;
    case Qt::XButton2:    return 4;
    default: assert(false); return 0;
    }
}

MouseEvent::PressPositions::PressPositions()
{
    for(int i = 0; i < 5; ++i)
    {
        x[i] = y[i] = -1;
        origin[i] = PressOriginNone;
    }
}

QPoint MouseEvent::PressPositions::Pos(int mouseButton) const
{
    int i = QtMouseButtonEnumToIndex(mouseButton);
    return QPoint(x[i], y[i]);
}

void MouseEvent::PressPositions::Set(int mouseButton, int x_, int y_, PressOrigin origin_)
{

    int i = QtMouseButtonEnumToIndex(mouseButton);
    x[i] = x_;
    y[i] = y_;
    origin[i] = origin_;
}

MouseEvent::PressOrigin MouseEvent::PressPositions::Origin(int mouseButton) const
{
    int i = QtMouseButtonEnumToIndex(mouseButton);
    return origin[i];
}

QPoint MouseEvent::MousePressedPos(int mouseButton) const
{
    return mousePressPositions.Pos(mouseButton);
}

bool MouseEvent::IsButtonDown(MouseButton button_) const
{
    return button == button_ || ((otherButtons & button_) != 0);
}
