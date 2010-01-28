// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ConsoleUIManager_h
#define incl_UiModule_ConsoleUIManager_h

#include <QObject>
#include <QPropertyAnimation>
#include "Framework.h"

namespace Ui
{
    class ConsoleWidget;
}

class QString;
class QRectF;

namespace CoreUi
{
    class ConsoleProxyWidget;

    class ConsoleUIManager: public QObject
    {
        Q_OBJECT

    public:
        ConsoleUIManager(Foundation::Framework *framework, QGraphicsView *ui_view);
        virtual ~ConsoleUIManager();

    public slots:

        //!Togge console on/off
        void ToggleConsole();
        
        //! slot to resize the console
        void SceneRectChanged(const QRectF &);

        //! prints the string to the console
        void PrintLine(const std::string &string);

        //!Handle input to the console(called after return pushed)
        void HandleInput();
        //! sends event to notify console module that UI for console is ready
        void SendInitializationReadyEvent();


    signals:
        void CommandIssued(const QString &string);

    private:
        //! configure the scrolling animation of the console
        void SetupAnimation();

        //! is console visible or hidden?
        bool visible_;


        //!Method for coloring/styling the string for the console
        void StyleString(QString &str);

        //Console opacity
        qreal opacity_;

        
        /// Event manager.
        Foundation::EventManagerPtr eventManager_;

        //Console event category
        event_category_id_t console_category_id_;

        //Framework pointer
        Foundation::Framework* framework_;

        //View to the scene
        QGraphicsView *ui_view_;

        //UI
        Ui::ConsoleWidget* console_ui_;

        //widget in UI
        QWidget * console_widget_;

        //proxy for our UI
        ConsoleProxyWidget *proxy_widget_;
        
        //! animation used for sliding effect
        QPropertyAnimation animation_;

        //!
        

    };


}
#endif