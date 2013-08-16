// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include <QTimer>
#include <QApplication>
#include <QStringList>

class QDir;
class QGraphicsView;
class QTranslator;
class QSplashScreen;

class Framework;

/// Represents the subclassed instance of the Qt's QApplication singleton.
class TUNDRACORE_API Application : public QApplication
{
    Q_OBJECT

    Q_PROPERTY(QString currentWorkingDirectory READ CurrentWorkingDirectory WRITE SetCurrentWorkingDirectory) /**< @copydoc CurrentWorkingDirectory */
    Q_PROPERTY(QString installationDirectory READ InstallationDirectory)    /**< @copydoc InstallationDirectory */
    Q_PROPERTY(QString userDataDirectory READ UserDataDirectory)            /**< @copydoc UserDataDirectory */
    Q_PROPERTY(QString userDocumentsDirectory READ UserDocumentsDirectory)  /**< @copydoc UserDocumentsDirectory */

    Q_PROPERTY(QString organizationName READ OrganizationName)  /**< @copydoc OrganizationName */
    Q_PROPERTY(QString applicationName READ ApplicationName)    /**< @copydoc ApplicationName */
    Q_PROPERTY(QString fullIdentifier READ FullIdentifier)      /**< @copydoc FullIdentifier */
    Q_PROPERTY(QString platform READ Platform)                  /**< @copydoc Platform */
        
    Q_PROPERTY(QString version READ Version)                    /**< @copydoc Version */
    Q_PROPERTY(uint majorVersion READ MajorVersion)             /**< @copydoc MajorVersion */
    Q_PROPERTY(uint minorVersion READ MinorVersion)             /**< @copydoc MinorVersion */
    Q_PROPERTY(uint majorPatchVersion READ MajorPatchVersion)   /**< @copydoc MajorPatchVersion */
    Q_PROPERTY(uint minorPatchVersion READ MinorPatchVersion)   /**< @copydoc MinorPatchVersion */

    Q_PROPERTY(double targetFpsLimit READ TargetFpsLimit WRITE SetTargetFpsLimit) /**< @copydoc TargetFpsLimit */
    Q_PROPERTY(double targetFpsLimitWhenInactive READ TargetFpsLimitWhenInactive WRITE SetTargetFpsLimitWhenInactive); /**< @copydoc TargetFpsLimitWhenInactive */

public:
    /// Constructs the application singleton.
    /** Qt requires its own data copy of the argc/argv parameters, so this object
        caches them. Pass in received command-line parameters here.
        @note Remember to call Initialize after construction. */
    Application(int &argc, char **argv);
    ~Application();

    /// Performs initialization that requires existence of the framework and stores the pointer.
    void Initialize(Framework *fw);

    virtual bool notify(QObject *receiver, QEvent *e);

    /// Request exit. Emits ExitRequested signal which can be used to cancel exit from Framework.
    void RequestExit();

    /// Starts the application execution.
    void Go();

    /// Displays a message to the user. Should be used when there is no usable window for displaying messages to the user.
    static void Message(const char *title, const char *text);
    static void Message(const std::string &title, const std::string &text); /**< @overload */
    static void Message(const wchar_t *title, const wchar_t *text); /**< @overload */
    static void Message(const std::wstring &title, const std::wstring &text); /**< @overload */

    /// Opens up a text console window for Tundra.
    /** @param attachToParent If true, the console is taken from the parent process. Use this to show Tundra console on
               the same console than the command line.
        @return True on success or if console already visible, false on failure. */
    static bool ShowConsoleWindow(bool attachToParent);

    /// Sets the current working directory. Use with caution.
    static void SetCurrentWorkingDirectory(QString newCwd);

    /// Returns the cwd of the current environment.
    /** This directory should not be relied, since it might change due to external code running.
        Always prefer to use InstallationDirectory, UserDataDirectory and UserDocumentsDirectory instead.
        The returned path contains a trailing slash. */
    static QString CurrentWorkingDirectory();

    /// Returns the directory where Tundra was installed to.
    /** This is *always* the directory Tundra.exe resides in.
        E.g. on Windows 7 this is usually of form "C:\Program Files (x86)\Tundra 1.0.5\".
        The returned path contains a trailing slash. */
    static QString InstallationDirectory();

    /// Returns the directory that is used for Tundra data storage on the current user.
    /** E.g. on Windows 7 this is usually of form "C:\Users\username\AppData\Roaming\Tundra\".
        The returned path contains a trailing slash. */
    static QString UserDataDirectory();

    /// Returns the directory where the documents (for Tundra) of the current user are located in.
    /** E.g. on Windows 7 this is usually of form "C:\Users\username\Documents\Tundra\".
        The returned path contains a trailing slash. */
    static QString UserDocumentsDirectory();

    /// Parse a filename for specific wildcard modifiers, and return as parsed
    /** $(CWD) is expanded to the current working directory.
        $(INSTDIR) is expanded to the Tundra installation directory (Application::InstallationDirectory)
        $(USERDATA) is expanded to Application::UserDataDirectory.
        $(USERDOCS) is expanded to Application::UserDocumentsDirectory.
        $(DATE:format) is expanded to show the current time, in this format http://doc.qt.nokia.com/latest/qdatetime.html#toString . */
    static QString ParseWildCardFilename(const QString& input);

    /// Return organization of the application, e.g. "realXtend".
    /** Returns C string as this information needs to be accessible without memory allocation for Windows minidump generation. */
    static const char *OrganizationName();

    /// Returns name of the application, "Tundra" usually.
    /** Returns C string as this information needs to be accessible without memory allocation for Windows minidump generation. */
    static const char *ApplicationName();

    /// Returns version information of the application as string, e.g. "2.0.0".
    /** Returns C string as this information needs to be accessible without memory allocation for Windows minidump generation. */
    static const char *Version();

    /// Returns the major version.
    uint MajorVersion() const { return versionNumbers[0]; }

    /// Returns the minor version.
    uint MinorVersion() const { return versionNumbers[1]; }

    /// Returns the major patch version.
    uint MajorPatchVersion() const { return versionNumbers[2]; }

    /// Returns the minor patch version.
    uint MinorPatchVersion() const { return versionNumbers[3]; }

    /// Returns the operating system/platform identifier.
    /** Returns 'win' for Windows, 'mac' for Mac OS X, 'x11' for Linux, 'android' for Android, and empty string otherwise.
        This is intended for scripting languages, as sometimes you need to do OS-specific UI changes with Qt etc. */
    static QString Platform();

    /// Returns "OrganizationName ApplicationName Version".
    /** @note Unlike OrganizationName, ApplicationName and Version, this function performs memory allocation. */
    static QString FullIdentifier();

    /// Specifies a new FPS limit to use for the main loop.
    /** Pass in a value of 0 to remove fps limiting altogether. */
    void SetTargetFpsLimit(double fpsLimit) { targetFpsLimit = fpsLimit; if (targetFpsLimit <= 1.0) targetFpsLimit = 0.0; }

    /// Returns the current FPS limit.
    /** 0 means the FPS limiting is disabled. */
    double TargetFpsLimit() const { return targetFpsLimit; }

    /// Sets a FPS limit to be used when the window is inactive
    void SetTargetFpsLimitWhenInactive(double fpsWhenInactive);

    /// Returns the current FPS limit when inactive.
    double TargetFpsLimitWhenInactive() const { return targetFpsLimitWhenInactive; }

    /// Reads and applies target FPS limit from config file.
    void ReadTargetFpsLimitFromConfig();

    /// Find .qm translation files from @c dir.
    QStringList FindQmFiles(const QDir &dir);

public slots:
    /// Returns if application is active.
    /** @note This is set to false when for example the main window is not focused. */
    bool IsActive() const;

    /// Update application.
    void UpdateFrame();

    /// Change language to input translation .qm @c file
    void ChangeLanguage(const QString& file);

    /// Set splash message.
    /** @note Only use this function during Tundra startup. */
    void SetSplashMessage(const QString &message);

signals:
    /// This signal is sent when QApplication language is changed, provided for convenience.
    void LanguageChanged();

    /// This signal is sent when the main window is closed or framework exit has been requested.
    void ExitRequested();

protected:
    /// QObject override.
    bool eventFilter(QObject *obj, QEvent *event);

private:
    /// Initializes splash screen.
    void InitializeSplash();

    Framework *framework;
    bool appActivated;
    QSplashScreen *splashScreen;
    QTimer frameUpdateTimer;
    QTranslator *nativeTranslator;
    QTranslator *appTranslator;
    static const char *organizationName;
    static const char *applicationName;
    static const char *version;
    double targetFpsLimit;
    double targetFpsLimitWhenInactive;

    uint versionNumbers[4];
};
