#include "StableHeaders.h"
#include "ConsoleUIManager.h"
#include "ui_ConsoleWidget.h"
#include "ConsoleProxyWidget.h"
#include <QGraphicsView>
#include <QString>
#include <QRegExp>
#include <QRectF>

#include <ConsoleEvents.h>
#include <ConsoleManager.h>
#include <ConsoleModule.h>



namespace CoreUi
{
    
    ConsoleUIManager::ConsoleUIManager(Foundation::Framework *framework, QGraphicsView *ui_view):
        framework_(framework),
        ui_view_(ui_view),
        console_ui_(new Ui::ConsoleWidget()),
        console_widget_( new QWidget()),
        visible_(false),
        opacity_(0.8)
    {
        console_ui_->setupUi(console_widget_);
        proxy_widget_ = new ConsoleProxyWidget(console_widget_);
        ui_view_->scene()->addItem(proxy_widget_);
        proxy_widget_->setOpacity(opacity_);

        QObject::connect(proxy_widget_, SIGNAL(TConsoleButtonPressed()), this, SLOT(ToggleConsole()));
        
        QObject::connect(console_ui_->ConsoleInputArea, SIGNAL(returnPressed()), this, SLOT(HandleInput()));
        
        eventManager_=framework_->GetEventManager();

        if(eventManager_.get())
        {
            console_category_id_ = eventManager_->QueryEventCategory("Console");
        }
        

        proxy_widget_->setMinimumHeight(0);
        proxy_widget_->setGeometry(QRect(0,0, ui_view_->width(),0));

        connect(ui_view_->scene(), SIGNAL( sceneRectChanged(const QRectF &) ),
        this, SLOT( SceneRectChanged(const QRectF &) ));


        SetupAnimation();
    }

    void ConsoleUIManager::SetupAnimation()
    {
        animation_.setTargetObject(proxy_widget_);
        animation_.setPropertyName("geometry");
        animation_.setDuration(500);
    }

    ConsoleUIManager::~ConsoleUIManager()
    {
        SAFE_DELETE(console_ui_);
        SAFE_DELETE(console_widget_);
    }

    void ConsoleUIManager::SendInitializationReadyEvent()
    {
        if(eventManager_.get())
        {
            
            Console::ConsoleEventData *event_data =  new Console::ConsoleEventData("");
            // send delayed because immediate message doesn't seem to go through to console
            eventManager_->SendDelayedEvent(console_category_id_, Console::Events::EVENT_CONSOLE_CONSOLE_VIEW_INITIALIZED, Foundation::EventDataPtr(event_data), 5);
        }
    }

    void ConsoleUIManager::HandleInput()
    {
        if(eventManager_.get())
        {
            QString text = console_ui_->ConsoleInputArea->text();
            Console::ConsoleEventData event_data(text.toStdString());
            eventManager_->SendEvent(console_category_id_, Console::Events::EVENT_CONSOLE_COMMAND_ISSUED, &event_data);
            console_ui_->ConsoleInputArea->clear();
        }
    }

    void ConsoleUIManager::PrintLine(const std::string &string)
    {
        QString str(string.c_str());
        StyleString(str);
        console_ui_->ConsoleTextArea->appendHtml(str);
    }

    void ConsoleUIManager::SceneRectChanged(const QRectF& rect)
    {
        if(visible_)
        {
            QRectF new_size = rect;
            new_size.setHeight(rect.height()*proxy_widget_->GetConsoleRelativeHeight());
            proxy_widget_->setGeometry(new_size);
        }
    }

    void ConsoleUIManager::ToggleConsole()
    {
        visible_ = !visible_;
        
        
        if(visible_)
        {
            animation_.setStartValue(QRect(0,0,ui_view_->width(),0));
            animation_.setEndValue(QRect(0,0,ui_view_->width(), ui_view_->height()*proxy_widget_->GetConsoleRelativeHeight()));
            
        }
        else
        {
            animation_.setStartValue(QRect(0,0,ui_view_->width(), ui_view_->height()*proxy_widget_->GetConsoleRelativeHeight()));
            animation_.setEndValue(QRect(0,0,ui_view_->width(),0));
        }
        this->console_ui_->ConsoleInputArea->setFocus();
        animation_.start();
        this->proxy_widget_->setActive(visible_);
        
    }

    void ConsoleUIManager::StyleString(QString &str)
    {
        QRegExp regexp;

        regexp.setPattern(".*Debug:.*");
        if(regexp.exactMatch(str))
        {
            str.push_front("<FONT COLOR=\"#FFFFFF\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Notice:.*");
        if(regexp.exactMatch(str))
        {
            str.push_front("<FONT COLOR=\"#0000FF\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Warning:.*");
        if(regexp.exactMatch(str))
        {
            str.push_front("<FONT COLOR=\"#CCCC00\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Error:.*");
        if(regexp.exactMatch(str))
        {
            str.push_front("<FONT COLOR=\"#FF3300\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Critical:.*");
        if(regexp.exactMatch(str))
        {
            str.push_front("<FONT COLOR=\"#FF0000\">");
            str.push_back("</FONT>");
            return;
        }


        regexp.setPattern(".*Fatal:.*");
        if(regexp.exactMatch(str))
        {
            str.push_front("<FONT COLOR=\"#9933CC\">");
            str.push_back("</FONT>");
            return;
        }
    }

}
