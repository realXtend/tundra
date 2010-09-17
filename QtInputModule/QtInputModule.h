// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtInputModule_QtInputModule_h
#define incl_QtInputModule_QtInputModule_h

#include "IModule.h"
#include "ModuleLoggingFunctions.h"
#include "IEventData.h"

#include "QtInputModuleApi.h"

#include "QtInputKeyEvent.h"
#include "QtInputMouseEvent.h"
#include "QtInputService.h"
#include "InputContext.h"

#include "InputServiceInterface.h"

#include <vector>
#include <list>
#include <QObject>
#include <QPointer>

class QGraphicsItem;
class QGraphicsView;
class QWidget;

class KeyBindingsConfigWindow;

/// QtInputModule provides other modules with different methods of acquiring keyboard and mouse input.
/** The input module works with the notion of 'input contexts', which are objects that modules acquire
    to receive input events. The contexts have a priority that determine the order in which the input 
    events are posted.

    Input events are processed in the following order:
        1) Very first, when a new input event is received, it is posted to the top level input context.
           See QtInputModule::TopLevelInputContext(). This is already before any Qt widgets get the
           chance to process the event.
        2) If the event is a mouse event that occurs on top of a Qt widget, or the event is a key event
           and a Qt widget has keyboard focus, the event is passed to Qt, and suppressed from going
           further.
        3) The event is posted to all the registered input contexts in their order of priority. See 
           QtInputModule::RegisterInputContext().
        4) The event is posted to the system-wide event tree. See the QtInputEvents namespace.

    At any level, the handler may set the handled member of a KeyEvent or MouseEvent to true to suppress
    the event from going forward to the lower levels.

    In addition to the abovementioned methods, a module may use a polling API to directly query input 
    state. This API operates on the input level (1) above. See the functions IsKeyDown, IsKeyPressed, 
    IsKeyReleased, IsMouseButtonDown, IsMouseButtonPressed and IsMouseButtonReleased.

    The InputContext -based API utilizes Qt signals. The polling API can be used by any object that
    has access to QtInputModule, and the event tree -based API can be used by all modules.
*/
class QTINPUT_MODULE_API QtInputModule : public QObject, public IModule
{
    Q_OBJECT

    MODULE_LOGGING_FUNCTIONS
    static const std::string &NameStatic();

public:
    QtInputModule();
    ~QtInputModule();

    void Initialize();
    void PostInitialize();
    void Update(f64 frametime);

    void ShowBindingsWindow();

    Console::CommandResult ShowBindingsWindowConsole(const StringVector &params);

public slots:
    void BindingsWindowClosed();

private:
    boost::shared_ptr<QtInputService> inputService;

    QPointer<KeyBindingsConfigWindow> configWindow;
};

#endif
