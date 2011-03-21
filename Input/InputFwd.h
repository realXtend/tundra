// For conditions of distribution and use, see copyright notice in license.txt

#ifndef Input_InputFwd_h
#define Input_InputFwd_h

#include <boost/shared_ptr.hpp>

class InputAPI;
class InputContext;
struct KeyPressInformation;
class KeyEvent;
class KeyEventSignal;
class MouseEvent;

class QGraphicsItem;
class QGraphicsView;
typedef boost::shared_ptr<InputContext> InputContextPtr;

#endif
