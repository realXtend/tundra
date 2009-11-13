// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_RexQEngine_moc_h
#define incl_Foundation_RexQEngine_moc_h

#include <QObject>
#include <QTimer>

class QApplication;
class QFrame;
class QHBoxLayout;
class OgreUIView;
class QGraphicsScene;

namespace Foundation
{

class Framework;

/// Bridges QtApplication and Rex Framework objects together and helps drive the continuous
/// application update ticks using a timer object.

class RexQEngine : public QObject
{
    Q_OBJECT

public:
    RexQEngine(Framework *owner, int &argc, char** argv);
    virtual ~RexQEngine();

    /// Transitions into the QApplication main loop.

    void Go();

    std::string GetMainWindowHandle() const;

    QWidget *GetMainWindowQWidget() const;

    QApplication *GetQApplication() const;

    void SendQAppQuitMessage();

public slots:

    void OnAboutToQuit();

    /// Responds to the frameupdate timer ticks by doing a logic and render step.

    void UpdateFrame();

private:
    QApplication *app_;

    Framework *owner_;
    QTimer frame_update_timer_;

//    QWidget *ogre_host_widget_;

//    QWidget *ogre_frame_;
//    QHBoxLayout *box_layout_;

    /// Transitions to true when the QApplication is being torn down and the application
    /// is about to quit.
    bool exit_signalled_;
};

}

#endif
