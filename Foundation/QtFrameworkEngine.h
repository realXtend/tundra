// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_QtFrameworkEngine_h
#define incl_Foundation_QtFrameworkEngine_h

#include "ForwardDefines.h"

#include <QObject>
#include <QTimer>
#include <QApplication>
#include <QGraphicsView>

namespace Foundation
{
    class Framework;

    /// Bridges QtApplication and Framework objects together and helps drive the continuous
    /// application update ticks using a timer object.
    /// Owns QApplication and QGraphicsView (main window) for its lifetime.

    class QtFrameworkEngine : public QObject
    {
    
    Q_OBJECT

    public:
        QtFrameworkEngine(Framework *owner, int &argc, char** argv);
        virtual ~QtFrameworkEngine();

        void Go();

        QApplication *GetQApplication() const;
        void SendQAppQuitMessage();

        QGraphicsView *GetUIView() const;
        void SetUIView(std::auto_ptr <QGraphicsView> view);

        void AddUIViewListener (QObject *listener);

    public slots:

        void OnAboutToQuit();
        void UpdateFrame();

    private:
        std::auto_ptr <QApplication>        app_;
        std::auto_ptr <QGraphicsView>       view_;

        QTimer frame_update_timer_;

        Framework *owner_;
        bool exit_signalled_;

    };

}

#endif
