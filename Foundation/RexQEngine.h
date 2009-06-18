#ifndef incl_Foundation_RexQEngine_moc_h
#define incl_Foundation_RexQEngine_moc_h

#include <QObject>
#include <QTimer>

class QApplication;
namespace Foundation
{

class Framework;

/// Bridges QtApplication and Rex Framework objects together and helps drive the continuous
/// application update ticks using a timer object.

class RexQEngine : public QObject
{
    Q_OBJECT

public:
    RexQEngine(Framework *owner);
    virtual ~RexQEngine();

    /// Transitions into the QApplication main loop.

    void Go();

    std::string GetMainWindowHandle() const;

public slots:

    void OnQuitQApp();

    /// Responds to the frameupdate timer ticks by doing a logic and render step.

    void UpdateFrame();

private:
    QApplication *app_;

    Framework *owner_;
    QTimer frame_update_timer_;


    QWidget *ogre_host_widget_;

    /// Transitions to true when the QApplication is being torn down and the application
    /// is about to quit.

    bool exit_signalled_;
};

}

#endif
