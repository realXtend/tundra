// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QWidget>

#include "CoreTypes.h"

class Framework;
class UiProxyWidget;

class QRectF;
class QGraphicsView;
class QPropertyAnimation;
class QPlainTextEdit;
class QLineEdit;

/// The UI window for the dropdown system console.
class ConsoleWidget: public QWidget
{
    Q_OBJECT

public:
    explicit ConsoleWidget(Framework *fw);
    virtual ~ConsoleWidget();

public slots:
    /// Prints to text to the console UI.
    /** @param text Text to be shown. */
    void PrintToConsole(const QString &text);

    /// Toggles visibility of the console UI.
    void ToggleConsole();

    /// Clears the console log.
    void ClearLog();

private slots:
    /// Handles input to the console (called after return is pressed).
    void HandleInput();

    /// Resize the console to fit scene
    void AdjustToSceneRect(const QRectF &);

private:
    /// QObject override.
    /** Used to filter console line edit keypresses in order to browse command history. */
    bool eventFilter(QObject *obj, QEvent *e);

    /// Decorates string for the console
    void DecorateString(QString &str);

    QPlainTextEdit *textEdit; //< Text area for the log.
    QLineEdit *lineEdit; //< Line edit for input.
    Framework* framework; ///< Framework pointer
    QGraphicsView *graphicsView; ///< View to the scene
    UiProxyWidget *proxyWidget; ///< Proxy for the UI
    QPropertyAnimation *slideAnimation; ///< Animation used for sliding effect
    QList<QString> commandHistory; ///< Command history stack.
    int commandHistoryIndex; ///< Current command history index.
    QString commandStub; ///< Current command stub used for autocompletion/suggestion
    QStringList prevSuggestions; ///< Already shown suggestions.
    float height; ///< Relative height [0,1] of the console UI.
};
