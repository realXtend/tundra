// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <boost/shared_ptr.hpp>

class InputAPI;
class InputContext;
struct KeyPressInformation;
class KeyEvent;
class KeyEventSignal;
class MouseEvent;
class GestureEvent;

class QGraphicsItem;
class QGraphicsView;
typedef boost::shared_ptr<InputContext> InputContextPtr;
