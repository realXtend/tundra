// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include <QTimer>
#include <QApplication>
#include <QStringList>

class QDir;
class QGraphicsView;
class QTranslator;

class Framework;

/// Represents the subclassed instance of the Qt's QApplication singleton.
class Application : public QApplication
{
    Q_OBJECT

public:
    /** Qt requires its own data copy of the argc/argv parameters, so this object
        caches them. Pass in received command-line parameters here.
        @param owner Pass in the root framework pointer here. */
    Application(Framework *owner, int &argc, char **argv);

    ~Application();

    virtual bool notify(QObject *receiver, QEvent *e);

    void Go();

    /// Displays a message to the user. Should be used when there is no usable window for displaying messages to the user.
    static void Message(const std::string &title, const std::string &text);

    /// Displays a message to the user. Should be used when there is no usable window for displaying messages to the user.
    static void Message(const std::wstring &title, const std::wstring &text);

    static void SetCurrentWorkingDirectory(QString newCwd);

    /// Returns the cwd of the current environment. This directory should not be relied, since it might change due to external
    /// code running. Always prefer to use InstallationDirectory, UserDataDirectory and UserDocumentsDirectory instead.
    /// The returned path contains a trailing slash.
    static QString CurrentWorkingDirectory();

    /// Returns the directory where Tundra was installed to. This is *always* the directory Tundra.exe resides in.
    /// E.g. on Windows 7 this is usually of form "C:\Program Files (x86)\Tundra 1.0.5\".
    /// The returned path contains a trailing slash.
    static QString InstallationDirectory();

    /// Returns the directory that is used for Tundra data storage on the current user.
    /// E.g. on Windows 7 this is usually of form "C:\Users\username\AppData\Roaming\Tundra\".
    /// The returned path contains a trailing slash.
    static QString UserDataDirectory();

    /// Returns the directory where the documents (for Tundra) of the current user are located in.
    /// E.g. on Windows 7 this is usually of form "C:\Users\username\Documents\Tundra\".
    /// The returned path contains a trailing slash.
    static QString UserDocumentsDirectory();

public slots:
    void UpdateFrame();
    void ChangeLanguage(const QString& file);
    void AboutToExit();

signals:
    /// This signal is sent when QApplication language is changed, provided for convenience.
    void LanguageChanged();

    /// This signal is sent when the main window is closed or framework exit has been requested.
    void ExitRequested();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QStringList GetQmFiles(const QDir &dir);
    Framework *framework;
    QTimer frameUpdateTimer;
    bool appActivated;
    QTranslator *nativeTranslator;
    QTranslator *appTranslator;
    int argc; ///< Command line argument count as supplied by the operating system.
    char **argv; ///< Command line arguments as supplied by the operating system.
};

