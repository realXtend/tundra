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

    // Console UI settings.
    /// @todo Settings for colors?
    const bool defaultInputEnabled = true;
    const float defaultHeight = 0.5f;
    const float defaultOpacity = 0.8f;
    const int defaultAnimationSpeed = 300;
    const int defaultBufferSize = 5000;

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
        cfg.Set(consoleUi, "buffer_size", defaultBufferSize);
    }

    bool inputEnabled = cfg.Get(consoleUi, "input_enabled", defaultInputEnabled).toBool();
    height = Clamp(cfg.Get(consoleUi, "relative_height", defaultHeight).toFloat(), 0.f, 1.f);
    float opacity = Clamp(cfg.Get(consoleUi, "opacity", defaultOpacity).toFloat(), 0.f, 1.f);
    int animationSpeed = cfg.Get(consoleUi, "animation_speed", defaultAnimationSpeed).toInt();
    int bufferSize = cfg.Get(consoleUi, "buffer_size", defaultBufferSize).toInt();

    if (fw->HasCommandLineParameter("--noconsoleguiinput"))
        inputEnabled = false; // Command line parameter authorative.

    setStyleSheet(QString(
        "QWidget {"
            "background-color: transparent;"
        "}"
        "QPlainTextEdit, QLineEdit {"
            "background-color: rgba(0,0,0,%1);"
            "font-family: \"Courier New\";"
            "font-size: 10pt;"
            "color: rgb(230,230,230);"
            "border: 0px;"
            "padding: 0px;"
            "margin: 0px;"
            "white-space: pre;"
        "}"
        "QLineEdit {"
            "border-top: 1px solid rgba(230,230,230,%1);"
            "border-bottom: 1px solid rgba(230,230,230,%1);"
            "padding-left: 3px;"
        "}"
        "QScrollBar{"
            "background-color: rgba(0,0,0,%1);"
        "}").arg(static_cast<int>(opacity*255.0)));

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    setLayout(layout);

    proxyWidget = framework->Ui()->AddWidgetToScene(this, Qt::Widget);
    proxyWidget->setMinimumHeight(0);
    proxyWidget->setGeometry(QRect(0, 0, graphicsView->width(), 0));
    proxyWidget->setZValue(20000);

    connect(framework->Ui()->GraphicsScene(), SIGNAL(sceneRectChanged(const QRectF&)), SLOT(AdjustToSceneRect(const QRectF&)));

    slideAnimation = new QPropertyAnimation(this);
    slideAnimation->setTargetObject(proxyWidget);
    slideAnimation->setPropertyName("geometry");
    slideAnimation->setDuration(animationSpeed);

    textEdit = new QPlainTextEdit(this);
    textEdit->setTextInteractionFlags(Qt::TextBrowserInteraction);
    textEdit->setMaximumBlockCount(bufferSize);
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
        switch(static_cast<QKeyEvent*>(e)->key())
        {
        case Qt::Key_Up:
        {
            if (commandHistoryIndex+1 < commandHistory.size())
                lineEdit->setText(commandHistory[++commandHistoryIndex]);
            return true;
        }
        case Qt::Key_Down:
        {
            --commandHistoryIndex;
            if (commandHistoryIndex < 0)
                commandHistoryIndex = -1;

            // Irssi-like behavior: clear line edit if not browsing history and down is pressed
            if (commandHistoryIndex == -1)
                lineEdit->clear();
            else if (commandHistoryIndex > -1 && commandHistoryIndex < commandHistory.size()-1)
                lineEdit->setText(commandHistory[commandHistoryIndex]);
            return true;
        }
        case Qt::Key_Tab:
        {
            QString text = lineEdit->text().trimmed();
            if (!text.isEmpty())
            {
                if (text != commandStub && !prevSuggestions.contains(text, Qt::CaseInsensitive))
                {
                    commandStub = text;
                    prevSuggestions.clear();
                }

                QStringList suggestions;
                for(ConsoleAPI::CommandMap::const_iterator it = framework->Console()->Commands().begin();
                    it != framework->Console()->Commands().end(); ++it)
                {
                    if (it->first.startsWith(commandStub, Qt::CaseInsensitive))
                        suggestions << it->first;
                }

                // Clear previous suggestion so that we can "start over".
                if (!prevSuggestions.isEmpty() && suggestions == prevSuggestions)
                    prevSuggestions.clear();

                foreach(const QString &command, suggestions)
                {
                    if (!prevSuggestions.contains(command, Qt::CaseInsensitive))
                    {
                        lineEdit->setText(command);
                        prevSuggestions << command;
                        break;
                    }
                }

                return true;
            }
        }
        default:
            return QObject::eventFilter(obj, e);
        }
    }

    return QObject::eventFilter(obj, e);
}

void ConsoleWidget::DecorateString(QString &str)
{
    static QString preStart = "<pre>";
    static QString preEnd = "</pre>";
    static QString preStartError = "<pre style=\"color:#FF3300\">";
    static QString preStartWarning = "<pre style=\"color:#FFFF00\">";
    static QString preStartDebug = "<pre style=\"color:#999999\">";
    static QRegExp regExpError = QRegExp(".*Error:.*");
    static QRegExp regExpWarning = QRegExp(".*Warning:.*");
    static QRegExp regExpDebug = QRegExp(".*Debug:.*");

    if (regExpError.exactMatch(str))
        str.prepend(preStartError);
    else if (regExpWarning.exactMatch(str))
        str.prepend(preStartWarning);
    else if (regExpDebug.exactMatch(str))
        str.prepend(preStartDebug);
    else
        str.prepend(preStart);
    str.append(preEnd);
}
