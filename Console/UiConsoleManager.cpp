// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "UiConsoleManager.h"
#include "ui_ConsoleWidget.h"
#include "ConsoleProxyWidget.h"
#include "ConsoleAPI.h"

#include "UiAPI.h"
#include "UiProxyWidget.h"
#include "UiGraphicsView.h"
#include "Framework.h"

#include <QGraphicsView>
#include <QString>
#include <QRegExp>
#include <QRectF>

#include "MemoryLeakCheck.h"

UiConsoleManager::UiConsoleManager(Framework *fw) :
    framework(fw),
    graphicsView(fw->Ui()->GraphicsView()),
    consoleUi(0),
    consoleWidget(0),
    proxyWidget(0),
    visible(false),
    commandHistoryIndex(-1)
{
    if (framework->IsHeadless())
        return;

    consoleUi = new Ui::ConsoleWidget();
    consoleWidget = new QWidget();

    // Init internals
    consoleUi->setupUi(consoleWidget);

    proxyWidget = framework->Ui()->AddWidgetToScene(consoleWidget);
    proxyWidget->setMinimumHeight(0);
    proxyWidget->setGeometry(QRect(0, 0, graphicsView->width(), 0));
    /// \todo Opacity has no effect atm.
    proxyWidget->setOpacity(0.8); ///<\todo Read opacity from config?
    proxyWidget->setZValue(20000);

    connect(framework->Ui()->GraphicsScene(), SIGNAL(sceneRectChanged(const QRectF&)), SLOT(AdjustToSceneRect(const QRectF&)));

    // Init animation
    slideAnimation.setTargetObject(proxyWidget);
    slideAnimation.setPropertyName("geometry");
    slideAnimation.setDuration(300);  ///<\todo Read animation speed from config?

    connect(consoleUi->ConsoleInputArea, SIGNAL(returnPressed()), SLOT(HandleInput()));

    consoleUi->ConsoleInputArea->installEventFilter(this);
}

UiConsoleManager::~UiConsoleManager()
{
    // consoleWidget gets deleted by the scene it is in currently
    if (consoleUi)
        SAFE_DELETE(consoleUi);
}

void UiConsoleManager::PrintToConsole(const QString &text)
{
    if (framework->IsHeadless())
        return;
    QString html = Qt::escape(text);
    DecorateString(html);
    consoleUi->ConsoleTextArea->appendHtml(html);
}

void UiConsoleManager::ToggleConsole()
{
    if (framework->IsHeadless())
        return;
    if (!graphicsView)
        return;

    QGraphicsScene *current_scene = proxyWidget->scene();
    if (!current_scene)
        return;

    visible = !visible;
    int current_height = graphicsView->height()*0.5;
    if (visible)
    {
        slideAnimation.setStartValue(QRect(0, 0, graphicsView->width(), 0));
        slideAnimation.setEndValue(QRect(0, 0, graphicsView->width(), current_height));
        // Not bringing to front, works in UiProxyWidgets, hmm...
        current_scene->setActiveWindow(proxyWidget);
        current_scene->setFocusItem(proxyWidget, Qt::ActiveWindowFocusReason);
        consoleUi->ConsoleInputArea->setFocus(Qt::MouseFocusReason);
        proxyWidget->show();
    }
    else
    {
        slideAnimation.setStartValue(QRect(0, 0, graphicsView->width(), current_height));
        slideAnimation.setEndValue(QRect(0, 0, graphicsView->width(), 0));
        proxyWidget->hide();
    }

    slideAnimation.start();
}

void UiConsoleManager::HandleInput()
{
    if (framework->IsHeadless())
        return;

    QString cmd = consoleUi->ConsoleInputArea->text();
    framework->Console()->ExecuteCommand(cmd);
    consoleUi->ConsoleInputArea->clear();
    commandStub.clear();
    commandHistory.push_front(cmd);
}

void UiConsoleManager::AdjustToSceneRect(const QRectF& rect)
{
    if (visible)
    {
        QRectF new_size = rect;
        new_size.setHeight(rect.height() * 0.5); ///<\todo Read height from config?
        proxyWidget->setGeometry(new_size);
    }
    else
    {
        proxyWidget->hide();
    }
}

bool UiConsoleManager::eventFilter(QObject *obj, QEvent *e)
{
    if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(e);
        if (keyEvent)
        {
            switch(keyEvent->key())
            {
            case Qt::Key_Up:
            {
                if (commandHistoryIndex+1 < commandHistory.size())
                    consoleUi->ConsoleInputArea->setText(commandHistory[++commandHistoryIndex]);
                return true;
            }
            case Qt::Key_Down:
            {
                --commandHistoryIndex;
                if (commandHistoryIndex < 0)
                    commandHistoryIndex = -1;

                if (commandHistoryIndex == -1)
                    // Irssi-like behavior: clear line edit if not browsing history and down is pressed
                    consoleUi->ConsoleInputArea->clear();
                else if (commandHistoryIndex > -1 && commandHistoryIndex < commandHistory.size()-1)
                        consoleUi->ConsoleInputArea->setText(commandHistory[commandHistoryIndex]);
                return true;
            }
            case Qt::Key_Tab:
            {
                QString text = consoleUi->ConsoleInputArea->text().trimmed().toLower();
                if (!text.isEmpty())
                {
                    if (text != commandStub && !prevSuggestions.contains(text))
                    {
                        commandStub = text;
                        prevSuggestions.clear();
                    }

                    QStringList suggestions;
//                    std::pair<std::string, ConsoleCommandStruct> p;
/* ///\todo Regression, reimplement. -jj.
                    foreach(p, commandManager->Commands())
                    {
                        QString cmd(p.first.c_str());
                        if (cmd.startsWith(commandStub))
                            suggestions.push_back(cmd);
                    }
*/
                    if (!prevSuggestions.isEmpty() && suggestions == prevSuggestions)
                        prevSuggestions.clear(); // Clear previous suggestion so that we can "start over".

                    foreach(const QString &cmd, suggestions)
                        if (!prevSuggestions.contains(cmd))
                        {
                            consoleUi->ConsoleInputArea->setText(cmd);
                            prevSuggestions.push_back(cmd);
                            break;
                        }

                    return true;
                }
            }
            default:
                return QObject::eventFilter(obj, e);
            }
        }
    }

    return QObject::eventFilter(obj, e);
}

void UiConsoleManager::DecorateString(QString &str)
{
    // Make all timestamp + module name blocks white
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
