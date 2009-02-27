// For conditions of distribution and use, see copyright notice in license.txt

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
