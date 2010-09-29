// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogicModule_RexMovementInput_h
#define incl_RexLogicModule_RexMovementInput_h

#include "InputFwd.h"
#include <QObject>

namespace Foundation
{
    class Framework;
}

namespace RexLogic
{

/// RexMovementInput is a input controller object that listens to keyboard
/// and mouse input events in its own input context and moves the local
/// avatar accordingly.
class RexMovementInput : public QObject
{
    Q_OBJECT

public:
    /// Initializes the input system by creating a new input context, in 
    /// which avatar movement events are read.
    /// @param context The input context that this controller takes to
    ///     read input from.
    explicit RexMovementInput(Foundation::Framework *framework);

public slots:
    void HandleKeyEvent(KeyEvent *key);
    void HandleMouseEvent(MouseEvent *mouse);

private:
    Foundation::Framework *framework;

    InputContextPtr input;
};

} // ~RexLogic

#endif
