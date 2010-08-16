// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiConsoleManager.h"
#include "ui_ConsoleWidget.h"
#include "ConsoleProxyWidget.h"
#include "ConsoleEvents.h"
#include "ConsoleManager.h"
#include "ConsoleModule.h"

#include "EventManager.h"
#include "Framework.h"

#include <QGraphicsView>
#include <QString>
#include <QRegExp>
#include <QRectF>

#include "MemoryLeakCheck.h"

namespace Console
{
    UiConsoleManager::UiConsoleManager(Foundation::Framework *framework, QGraphicsView *ui_view) :
        framework_(framework),
        ui_view_(ui_view),
        console_ui_(new Ui::ConsoleWidget()),
        console_widget_(new QWidget()),
        visible_(false),
        opacity_(0.8)
    {
        // Init internals
        console_ui_->setupUi(console_widget_);
        proxy_widget_ = new ConsoleProxyWidget(console_widget_);
        proxy_widget_->setMinimumHeight(0);
        proxy_widget_->setGeometry(QRect(0, 0, ui_view_->width(), 0));
        proxy_widget_->setOpacity(opacity_);
        ui_view_->scene()->addItem(proxy_widget_);

        SetupAnimation();
        ConnectSignals();

        // Init event categories
        console_category_id_ = framework_->GetEventManager()->QueryEventCategory("Console");
    }

    UiConsoleManager::~UiConsoleManager()
    {
        SAFE_DELETE(console_ui_);
        SAFE_DELETE(console_widget_);
    }

    void UiConsoleManager::SetupAnimation()
    {
        animation_.setTargetObject(proxy_widget_);
        animation_.setPropertyName("geometry");
        animation_.setDuration(300);
    }

    void UiConsoleManager::ConnectSignals()
    {
        // Proxy show/hide toggle
//        connect(ui_view_, SIGNAL( ConsoleToggleRequest() ), 
//                this, SLOT( ToggleConsole() ));

        // Input field
        connect(console_ui_->ConsoleInputArea, SIGNAL( returnPressed() ), 
                this, SLOT( HandleInput() ));

        // Scene to notify rect changes
        connect(ui_view_->scene(), SIGNAL( sceneRectChanged(const QRectF &) ),
                this, SLOT( AdjustToSceneRect(const QRectF &) ));

        // Print queuing with Qt::QueuedConnection to avoid problems when printing from threads
        connect(this, SIGNAL( PrintOrderRecieved(QString) ), 
                this, SLOT( PrintToConsole(QString) ), 
                Qt::QueuedConnection);
    }

    void UiConsoleManager::SendInitializationReadyEvent()
    {
        Console::ConsoleEventData *event_data =  new Console::ConsoleEventData("");
        framework_->GetEventManager()->SendDelayedEvent(console_category_id_, Console::Events::EVENT_CONSOLE_CONSOLE_VIEW_INITIALIZED,
            Foundation::EventDataPtr(event_data), 1);
    }

    void UiConsoleManager::HandleInput()
    {
        QString text = console_ui_->ConsoleInputArea->text();
        Console::ConsoleEventData event_data(text.toStdString());
        framework_->GetEventManager()->SendEvent(console_category_id_, Console::Events::EVENT_CONSOLE_COMMAND_ISSUED, &event_data);
        console_ui_->ConsoleInputArea->clear();
    }

    void UiConsoleManager::QueuePrintRequest(const QString &text)
    {
        emit PrintOrderRecieved(text);
    }

    void UiConsoleManager::PrintToConsole(QString text)
    {
        QString html = Qt::escape(text);
        StyleString(html);
        console_ui_->ConsoleTextArea->appendHtml(html);
    }

    void UiConsoleManager::AdjustToSceneRect(const QRectF& rect)
    {

        if (visible_)
        {
            QRectF new_size = rect;
            new_size.setHeight(rect.height() * proxy_widget_->GetConsoleRelativeHeight());
            proxy_widget_->setGeometry(new_size);
        }
        else
        {
            proxy_widget_->hide();
        }
        
    }

    void UiConsoleManager::ToggleConsole()
    {
        if (!ui_view_)
            return;
        if (ui_view_->scene() != proxy_widget_->scene())
            return;

        visible_ = !visible_;
        int current_height = ui_view_->height()*proxy_widget_->GetConsoleRelativeHeight();
        if (visible_)
        {
            animation_.setStartValue(QRect(0, 0, ui_view_->width(), 0));
            animation_.setEndValue(QRect(0, 0, ui_view_->width(), current_height));
            // Not bringing to front, works in UiProxyWidgets, hmm...
            ui_view_->scene()->setActiveWindow(proxy_widget_);
            ui_view_->scene()->setFocusItem(proxy_widget_, Qt::ActiveWindowFocusReason);
            console_ui_->ConsoleInputArea->setFocus(Qt::MouseFocusReason);
            proxy_widget_->show();
        }
        else
        {
            animation_.setStartValue(QRect(0, 0, ui_view_->width(), current_height));
            animation_.setEndValue(QRect(0, 0, ui_view_->width(), 0));
            proxy_widget_->hide();
        }
        animation_.start();
    }

    void UiConsoleManager::StyleString(QString &str)
    {
        // Make all timespamp + module name blocks white
        int block_end_index = str.indexOf("]");
        if (block_end_index != -1)
        {
            QString span_start("<span style='color:white;'>");
            QString span_end("</span>");
            block_end_index += span_start.length() + 1;
            str.insert(0, "<span style='color:white;'>");
            str.insert(block_end_index, "</span>");
            block_end_index += span_end.length();
        }
        else
            block_end_index = 0;

        QRegExp regexp;
        regexp.setPattern(".*Debug:.*");
        if (regexp.exactMatch(str))
        {
            str.insert(block_end_index, "<FONT COLOR=\"#999999\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Notice:.*");
        if (regexp.exactMatch(str))
        {
            str.insert(block_end_index, "<FONT COLOR=\"#0000FF\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Warning:.*");
        if (regexp.exactMatch(str))
        {
            str.insert(block_end_index, "<FONT COLOR=\"#FFFF00\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Error:.*");
        if (regexp.exactMatch(str))
        {
            str.insert(block_end_index, "<FONT COLOR=\"#FF3300\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Critical:.*");
        if (regexp.exactMatch(str))
        {
            str.insert(block_end_index, "<FONT COLOR=\"#FF0000\">");
            str.push_back("</FONT>");
            return;
        }

        regexp.setPattern(".*Fatal:.*");
        if (regexp.exactMatch(str))
        {
            str.insert(block_end_index, "<FONT COLOR=\"#9933CC\">");
            str.push_back("</FONT>");
            return;
        }
    }

}
