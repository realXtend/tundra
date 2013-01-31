// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "ConsoleWidget.h"
#include "ConsoleAPI.h"

#include "UiAPI.h"
#include "UiProxyWidget.h"
#include "UiGraphicsView.h"
#include "Framework.h"
#include "ConfigAPI.h"
#include "Math/MathFunc.h"

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
    commandHistoryIndex(-1),
    lineEdit(0)
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

    // Console UI settings.
    /// @todo Settings for colors?
    const bool defaultInputEnabled = true;
    const float defaultHeight = 0.5f;
    const float defaultOpacity = 0.8f;
    const int defaultAnimationSpeed = 300;

    ConfigAPI &cfg = *framework->Config();
    ConfigData consoleUi(ConfigAPI::FILE_FRAMEWORK, "console_ui");
    // If we have "input_enabled" in config we very likely have all the other settings as well.
    // If not, write the default settings to config for the next time.
    if (!cfg.HasValue(consoleUi, "input_enabled"))
    {
        cfg.Set(consoleUi, "input_enabled", defaultInputEnabled);
        cfg.Set(consoleUi, "relative_height", (double)defaultHeight);
        cfg.Set(consoleUi, "opacity", (double)defaultOpacity);
        cfg.Set(consoleUi, "animation_speed", defaultAnimationSpeed);
    }

    bool inputEnabled = cfg.Get(consoleUi, "input_enabled", defaultInputEnabled).toBool();
    height = Clamp(cfg.Get(consoleUi, "relative_height", defaultHeight).toFloat(), 0.f, 1.f);
    float opacity = Clamp(cfg.Get(consoleUi, "opacity", defaultOpacity).toFloat(), 0.f, 1.f);
    int animationSpeed = cfg.Get(consoleUi, "animation_speed", defaultAnimationSpeed).toInt();

    if (fw->HasCommandLineParameter("--noconsoleguiinput"))
        inputEnabled = false; // Command line parameter authorative.

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    proxyWidget = framework->Ui()->AddWidgetToScene(this, Qt::Widget);
    proxyWidget->setMinimumHeight(0);
    proxyWidget->setGeometry(QRect(0, 0, graphicsView->width(), 0));
    proxyWidget->setOpacity(opacity);
    proxyWidget->setZValue(20000);

    connect(framework->Ui()->GraphicsScene(), SIGNAL(sceneRectChanged(const QRectF&)), SLOT(AdjustToSceneRect(const QRectF&)));

    slideAnimation = new QPropertyAnimation(this);
    slideAnimation->setTargetObject(proxyWidget);
    slideAnimation->setPropertyName("geometry");
    slideAnimation->setDuration(animationSpeed);

    textEdit = new QPlainTextEdit(this);
    textEdit->setTextInteractionFlags(Qt::TextSelectableByMouse);
    layout->addWidget(textEdit);

    if (inputEnabled)
    {
        lineEdit = new QLineEdit(this);
        connect(lineEdit, SIGNAL(returnPressed()), SLOT(HandleInput()));
        lineEdit->installEventFilter(this);
        layout->addWidget(lineEdit);
    }
}

ConsoleWidget::~ConsoleWidget()
{
    // consoleWidget gets deleted by the scene it is in currently
}

void ConsoleWidget::PrintToConsole(const QString &text)
{
    if (textEdit)
    {
        QString html = Qt::escape(text);
        DecorateString(html);
        textEdit->appendHtml(html);
    }
}

void ConsoleWidget::ToggleConsole()
{
    if (!graphicsView)
        return;

    QGraphicsScene *graphicsScene = proxyWidget->scene();
    if (!graphicsScene)
        return;

    proxyWidget->setVisible(proxyWidget->isVisible());
    int currentHeight = graphicsView->height() * height;
    if (!proxyWidget->isVisible())
    {
        slideAnimation->setStartValue(QRect(0, 0, graphicsView->width(), 0));
        slideAnimation->setEndValue(QRect(0, 0, graphicsView->width(), currentHeight ));
        // Not bringing to front, works in UiProxyWidgets, hmm...
        graphicsScene->setActiveWindow(proxyWidget);
        graphicsScene->setFocusItem(proxyWidget, Qt::ActiveWindowFocusReason);
        if (lineEdit)
            lineEdit->setFocus(Qt::MouseFocusReason);
        proxyWidget->show();
    }
    else
    {
        slideAnimation->setStartValue(QRect(0, 0, graphicsView->width(), currentHeight));
        slideAnimation->setEndValue(QRect(0, 0, graphicsView->width(), 0));
        proxyWidget->hide();
    }

    slideAnimation->start();
}

void ConsoleWidget::ClearLog()
{
    textEdit->clear();
}

void ConsoleWidget::HandleInput()
{
    if (framework->IsHeadless() || !lineEdit)
        return;

    QString cmd = lineEdit->text();
    framework->Console()->ExecuteCommand(cmd);
    lineEdit->clear();
    commandStub.clear();
    commandHistory.push_front(cmd);
}

void ConsoleWidget::AdjustToSceneRect(const QRectF& rect)
{
    if (proxyWidget->isVisible())
    {
        QRectF newSize = rect;
        newSize.setHeight(rect.height() * height);
        proxyWidget->setGeometry(newSize);
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
                    std::pair<QString, shared_ptr<ConsoleCommand> > p;
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
    int blockEndIndex = 0;
    // Code below disabled as we no longer have module+timestamp block.
/*
    int blockEndIndex = str.indexOf("]");
    if (blockEndIndex != -1)
    {
        QString span_start("<span style='color:white;'>");
        QString span_end("</span>");
        blockEndIndex += span_start.length() + 1;
        str.insert(0, "<span style='color:white;'>");
        str.insert(blockEndIndex, "</span>");
        blockEndIndex += span_end.length();
    }
    else
        blockEndIndex = 0;
*/
    QRegExp regexp;
    regexp.setPattern(".*Debug:.*");
    if (regexp.exactMatch(str))
    {
        str.insert(blockEndIndex, "<FONT COLOR=\"#999999\">");
        str.push_back("</FONT>");
        return;
    }
/*
    regexp.setPattern(".*Notice:.*");
    if (regexp.exactMatch(str))
    {
        str.insert(blockEndIndex, "<FONT COLOR=\"#0000FF\">");
        str.push_back("</FONT>");
        return;
    }
*/
    regexp.setPattern(".*Warning:.*");
    if (regexp.exactMatch(str))
    {
        str.insert(blockEndIndex, "<FONT COLOR=\"#FFFF00\">");
        str.push_back("</FONT>");
        return;
    }

    regexp.setPattern(".*Error:.*");
    if (regexp.exactMatch(str))
    {
        str.insert(blockEndIndex, "<FONT COLOR=\"#FF3300\">");
        str.push_back("</FONT>");
        return;
    }
/*
    regexp.setPattern(".*Critical:.*");
    if (regexp.exactMatch(str))
    {
        str.insert(blockEndIndex, "<FONT COLOR=\"#FF0000\">");
        str.push_back("</FONT>");
        return;
    }

    regexp.setPattern(".*Fatal:.*");
    if (regexp.exactMatch(str))
    {
        str.insert(blockEndIndex, "<FONT COLOR=\"#9933CC\">");
        str.push_back("</FONT>");
        return;
    }
*/
}
