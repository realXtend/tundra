// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_Application_h
#define incl_Foundation_Application_h

#include <QTimer>
#include <QString>
#include <QApplication>
#include <QStringList>

#include "HighPerfClock.h"

class QDir;
class QGraphicsView;
class QTranslator;
class QSplashScreen;

namespace Foundation { class Framework; }

/** Frameworks singleton Application that runs the Qt main loop and updates our frameworks main loop.
 
    Inherits QApplication.
*/
class Application : public QApplication
{
    Q_OBJECT

public:
    /// Constructor.
    /// \note Qt requires its own data copy of the argc/argv parameters, so NaaliApplication caches them. Pass in received command-line parameters here.
    /// \param Foundation::Framework* Framework pointer.
    /// \param int argc
    /// \param char** argv
    Application(Foundation::Framework *owner, int &argc, char **argv);

    /// Deconstructor.
    ~Application();

    /// QApplication override.
    virtual bool notify(QObject *receiver, QEvent *e);

    /// Go starts the Qt main loop with QApplications::exec().
    /// \note This function blocks, it returns only after Qt's main loop is stopped.
    void Go();

protected:
    virtual void timerEvent(QTimerEvent *event);

public slots:
    /// Process all Qt events and process framework frame.
    void UpdateFrame();

    /// Change language of the application to the passed language 
    /// \param QString Language identifier eg. ru, de, fr or es. See data/translations/qt_native_translations for available languages.
    void ChangeLanguage(const QString& file);

    /// Tells the Application to prepare exiting.
    /// \note This function emits ExitRequested() signal, you can cancel the exit with Foundation::Framework::CancelExit().
    void AboutToExit();

    /// Initialize splash screen.
    /// \note Does nothing in headless mode.
    void InitializeSplash();

    /// Sets the current message displayed in the splash screen on startup.
    /// \note Does nothing in headless mode or if Go() has been called.
    void SetSplashMessage(const QString &message);

private:
    bool RunFrameworkUpdate();
    void SetTargetFps(float fps);
    void RestoreTargetFps();

signals:
    /// This signal is sent when QApplication language is changed, provided for convenience.
    void LanguageChanged();

    /// This signal is sent when the main window is closed or framework exit has been requested.
    /// \note You can cancel the exit with Foundation::Framework::CancelExit().
    void ExitRequested();

protected:
    /// QObject override.
    bool eventFilter(QObject *obj, QEvent *event);

private:
    QStringList GetQmFiles(const QDir &dir);
    Foundation::Framework *framework;

    QSplashScreen *splashScreen;
    QTranslator *nativeTranslator;
    QTranslator *appTranslator;

    int fwUpdateId_;
    uint fwSkipFrames_;
    float targetFpsStartParam_;
    float targetFps_;
    tick_t lastPresentTime_;
    tick_t timerFrequency_;

    int argc; ///< Command line argument count as supplied by the operating system.
    char **argv; ///< Command line arguments as supplied by the operating system.
};

#endif
