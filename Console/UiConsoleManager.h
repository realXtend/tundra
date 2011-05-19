// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Console_UiConsoleManager_h
#define incl_Console_UiConsoleManager_h

#include <QObject>
#include <QPropertyAnimation>

#include "ui_ConsoleWidget.h"
#include "CoreTypes.h"

class Framework;

class QRectF;
class QGraphicsView;
class UiProxyWidget;
class CommandManager;

// @todo Rename to ConsoleWindow, remove ConsoleWidget.ui and implemented the widget fully in C++ code. Make ConsoleAPI work as "UiConsoleManager".
class UiConsoleManager: public QObject
{
    Q_OBJECT

public:
    explicit UiConsoleManager(/*CommandManager* mgr, */Framework *fw);
    virtual ~UiConsoleManager();

public slots:
    /// Prints to text to the console UI.
    /** @param text Text to be shown. */
    void PrintToConsole(const QString &text);

    /// Toggles visibility of the console UI.
    void ToggleConsole();

signals:
    /// This emit is Qt::QueuedConnection type to avoid issues when printing from threads
    void PrintOrderReceived(const QString &text);

    /// TODO: comment
    void CommandIssued(const QString &string);

private slots:
    /// Handles input to the console (called after return is pressed).
    void HandleInput();

    /// Resize the console to fit scene
    void AdjustToSceneRect(const QRectF &);

private:
    /// QObject override.
    /** Used to filter console line edit keypresses in order to browse command history. */
    bool eventFilter(QObject *obj, QEvent *e);

    void StyleString(QString &str);
    /// Decorates string for the console
    void DecorateString(QString &str);
    bool visible; ///< Is the console UI visible.
    Framework* framework; ///< Framework pointer
//    CommandManager *commandManager; ///< Command manager.
    QGraphicsView *graphicsView; ///< View to the scene
    Ui_ConsoleWidget* consoleUi; ///< UI
    QWidget * consoleWidget; ///< Widget in UI
    UiProxyWidget *proxyWidget; ///< Proxy for our UI
    QPropertyAnimation slideAnimation; ///< Animation used for sliding effect
    QList<QString> commandHistory; ///< Command history stack.
    int commandHistoryIndex; ///< Current command history index.
    QString commandStub; ///< Current command stub used for autocompletion/suggestion
    QStringList prevSuggestions; ///< Already shown suggestions.
};

#endif
