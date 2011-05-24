// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ConsoleWidget.h"
#include "ConsoleAPI.h"

#include "UiAPI.h"
#include "UiProxyWidget.h"
#include "UiGraphicsView.h"
#include "Framework.h"

#include <QPlainTextEdit>
#include <QGraphicsView>
#include <QString>
#include <QRegExp>
#include <QRectF>
#include <QPropertyAnimation>

#include "MemoryLeakCheck.h"

ConsoleWidget::ConsoleWidget(Framework *fw) :
    framework(fw),
    graphicsView(fw->Ui()->GraphicsView()),
    proxyWidget(0),
    visible(false),
    commandHistoryIndex(-1)
{
    setStyleSheet(
        "QPlainTextEdit {"
            "border: 0px;"
            "padding: 0px;"
            "background-color: rgb(0,0,0);"
            "font-size: 12px;"
            "color: #33CC00;"
            "margin: 0px;"
        "}"
        "QLineEdit {"
            "border: 0px;"
            "border-top: 1px solid rgba(255,255,255,100);"
            "border-bottom: 1px rgba(255,255,255,100);"
            "padding: 0px;"
            "padding-left: 3px;"
            "background-color: rgb(0,0,0);"
            "font-size: 12px;"
            "color:#33CC00;"
        "}"
        "QScrollBar{"
            "background-color: rgb(0,0,0);"
        "}");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    proxyWidget = framework->Ui()->AddWidgetToScene(this);
    proxyWidget->setMinimumHeight(0);
    proxyWidget->setGeometry(QRect(0, 0, graphicsView->width(), 0));
    /// \todo Opacity has no effect atm.
    proxyWidget->setOpacity(0.8); ///<\todo Read opacity from config?
    proxyWidget->setZValue(20000);

    connect(framework->Ui()->GraphicsScene(), SIGNAL(sceneRectChanged(const QRectF&)), SLOT(AdjustToSceneRect(const QRectF&)));

    // Init animation
    slideAnimation = new QPropertyAnimation(this);
    slideAnimation->setTargetObject(proxyWidget);
    slideAnimation->setPropertyName("geometry");
    slideAnimation->setDuration(300);  ///<\todo Read animation speed from config?

    textEdit = new QPlainTextEdit(this);
    textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
    lineEdit = new QLineEdit(this);
    connect(lineEdit, SIGNAL(returnPressed()), SLOT(HandleInput()));

    lineEdit->installEventFilter(this);

    layout->addWidget(textEdit);
    layout->addWidget(lineEdit);
}

ConsoleWidget::~ConsoleWidget()
{
    // consoleWidget gets deleted by the scene it is in currently
}

void ConsoleWidget::PrintToConsole(const QString &text)
{
    QString html = Qt::escape(text);
    DecorateString(html);
    textEdit->appendHtml(html);
}

void ConsoleWidget::ToggleConsole()
{
    if (!graphicsView)
        return;

    QGraphicsScene *current_scene = proxyWidget->scene();
    if (!current_scene)
        return;

    visible = !visible;
    int current_height = graphicsView->height()*0.5;
    if (visible)
    {
        slideAnimation->setStartValue(QRect(0, 0, graphicsView->width(), 0));
        slideAnimation->setEndValue(QRect(0, 0, graphicsView->width(), current_height));
        // Not bringing to front, works in UiProxyWidgets, hmm...
        current_scene->setActiveWindow(proxyWidget);
        current_scene->setFocusItem(proxyWidget, Qt::ActiveWindowFocusReason);
        lineEdit->setFocus(Qt::MouseFocusReason);
        proxyWidget->show();
    }
    else
    {
        slideAnimation->setStartValue(QRect(0, 0, graphicsView->width(), current_height));
        slideAnimation->setEndValue(QRect(0, 0, graphicsView->width(), 0));
        proxyWidget->hide();
    }

    slideAnimation->start();
}

void ConsoleWidget::HandleInput()
{
    if (framework->IsHeadless())
        return;

    QString cmd = lineEdit->text();
    framework->Console()->ExecuteCommand(cmd);
    lineEdit->clear();
    commandStub.clear();
    commandHistory.push_front(cmd);
}

void ConsoleWidget::AdjustToSceneRect(const QRectF& rect)
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

bool ConsoleWidget::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == lineEdit && e->type() == QEvent::KeyPress)
    {
        QKeyEvent *keyEvent = dynamic_cast<QKeyEvent*>(e);
        if (keyEvent)
        {
            switch(keyEvent->key())
            {
            case Qt::Key_Up:
                if (commandHistoryIndex+1 < commandHistory.size())
                    lineEdit->setText(commandHistory[++commandHistoryIndex]);
                return true;
            case Qt::Key_Down:
                --commandHistoryIndex;
                if (commandHistoryIndex < 0)
                    commandHistoryIndex = -1;

                if (commandHistoryIndex == -1)
                    // Irssi-like behavior: clear line edit if not browsing history and down is pressed
                    lineEdit->clear();
                else if (commandHistoryIndex > -1 && commandHistoryIndex < commandHistory.size()-1)
                        lineEdit->setText(commandHistory[commandHistoryIndex]);
                return true;
            case Qt::Key_Tab:
            {
                QString text = lineEdit->text().trimmed().toLower();
                if (!text.isEmpty())
                {
                    if (text != commandStub && !prevSuggestions.contains(text))
                    {
                        commandStub = text;
                        prevSuggestions.clear();
                    }

                    QStringList suggestions;
                    std::pair<QString, boost::shared_ptr<ConsoleCommand> > p;
                    foreach(p, framework->Console()->Commands())
                        if (p.first.toLower().startsWith(commandStub))
                            suggestions.push_back(p.first.toLower());

                    if (!prevSuggestions.isEmpty() && suggestions == prevSuggestions)
                        prevSuggestions.clear(); // Clear previous suggestion so that we can "start over".

                    foreach(const QString &cmd, suggestions)
                        if (!prevSuggestions.contains(cmd))
                        {
                            lineEdit->setText(cmd);
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

void ConsoleWidget::DecorateString(QString &str)
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
