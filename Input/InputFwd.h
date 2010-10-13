#ifndef Core_Input_InputFwd_h
#define Core_Input_InputFwd_h

#include <boost/shared_ptr.hpp>

class Input;
class InputContext;
struct KeyPressInformation;
class KeyEvent;
class KeyEventSignal;
class MouseEvent;

class QGraphicsItem;
class QGraphicsView;
typedef boost::shared_ptr<InputContext> InputContextPtr;

#endif
