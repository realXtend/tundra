#ifndef incl_UiModule_ConsoleProxyWidget_h
#define incl_UiModule_ConsoleProxyWidget_h

#include <QGraphicsProxyWidget>
#include "UiWidgetProperties.h"

class QKeyEvent;
class QEvent;
namespace CoreUi
{

    class ConsoleProxyWidget: public QGraphicsProxyWidget
    {
        Q_OBJECT
        
    public:
        //!overriden 
        void keyPressEvent(QKeyEvent* e);

        ConsoleProxyWidget(QWidget *widget);
        virtual ~ConsoleProxyWidget(void);
        

        //!returns the consoles relative height (value is between 0-1,0 is not visible, 1 is the whole screen)
        qreal GetConsoleRelativeHeight(){return con_relative_height_;}
        //! sets consoles relative height (0-1), if value is not between 0-1, it will be clamped to this value range
        void setConsoleRelativeHeight(qreal height);

    signals:
        //emitted when toggle console button is pressed
        void TConsoleButtonPressed();

    private:
        //! how much of the screen space console uses when dropped down (0 is not visible, 1 is the whole screen)
        qreal con_relative_height_;


    };

}
#endif