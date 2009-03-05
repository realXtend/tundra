// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_InputEventHandler_h
#define incl_InputEventHandler_h



class InputEventHandler
{
public:
    InputEventHandler();
    virtual ~InputEventHandler();
    
    void HandleLeftMouseClick();
    void HandleRightMouseClick();
    void HandleMouseMove();
    void HandleKeyboardEvent();

};
 

#endif
