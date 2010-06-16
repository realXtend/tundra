// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_InputServiceInterface_h
#define incl_Interfaces_InputServiceInterface_h

#include "QtInputKeyEvent.h"
#include "QtInputMouseEvent.h"
#include "InputContext.h"

#include "ServiceInterface.h"

class QGraphicsItem;
class QGraphicsView;
class QWidget;

class InputServiceInterface : public Foundation::ServiceInterface
{
public:
    InputServiceInterface() {}
    virtual ~InputServiceInterface() {}

	virtual QGraphicsItem *GetVisibleItemAtCoords(int x, int y) = 0;
    virtual void SetMouseCursorVisible(bool visible) = 0;
    virtual bool IsMouseCursorVisible() const = 0;
    virtual bool IsKeyDown(Qt::Key keyCode) const = 0;
    virtual bool IsKeyPressed(Qt::Key keyCode) const = 0;
    virtual bool IsKeyReleased(Qt::Key keyCode) const = 0;
    virtual bool IsMouseButtonDown(int mouseButton) const = 0;
    virtual bool IsMouseButtonPressed(int mouseButton) const = 0;
    virtual bool IsMouseButtonReleased(int mouseButton) const = 0;
    virtual QPoint MousePressedPos(int mouseButton) const = 0;
    virtual boost::shared_ptr<InputContext> RegisterInputContext(const char *name, int priority) = 0;
    virtual void TriggerKeyEvent(KeyEvent &key) = 0;
    virtual void TriggerMouseEvent(MouseEvent &mouse) = 0;
    virtual InputContext &TopLevelInputContext() = 0;
};

#endif
