

#ifndef __incl_InputEventHandler_h__
#define __incl_InputEventHandler_h__



class InputEventHandler
{
public:
    InputEventHandler();
    virtual ~InputEventHandler();
    
    void handleLeftMouseClick();
    void handleRightMouseClick();
    void handleMouseMove();
    void handleKeyboardEvent();

};
 

#endif
