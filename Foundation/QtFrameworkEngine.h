// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_QtFrameworkEngine_moc_h
#define incl_Foundation_QtFrameworkEngine_moc_h

#include "ForwardDefines.h"

#include <QObject>
#include <QKeyEvent>
#include <QTimer>
#include <QApplication>
#include <QGraphicsView>

namespace Foundation
{
    class Framework;

    /// Used for "unbuffered" keyboard state from "buffered" Qt Events
    /// currently owned by Qt engine, but may have a better home elsewhere
    struct KeyState
    {
        KeyState ();
        KeyState (int k, unsigned int m);
        KeyState (const KeyState &c);
        KeyState (const QKeyEvent *e);
        KeyState &operator= (const KeyState &rhs);
        bool operator== (const KeyState &rhs) const;
        bool operator== (int k) const;

        int key;
        unsigned int modifiers;
    };

    /// Install this even filter on the root window (QGraphicsView)
    /// to capture all keyboard events
    class KeyStateListener : public QObject
    {
        public:
            typedef std::vector <KeyState> KeyStateVector;
            typedef std::vector <char> CharStateVector;

            KeyStateListener ();

            const KeyStateVector &GetState (); 
            const CharStateVector GetCharState (); 

            bool IsKeyDown (const KeyState &key);
            bool IsKeyDown (int key, unsigned int modifiers = 0);

            static char KeyToAscii (const KeyState &k);
            static KeyState AsciiToKey (char c);

        protected:
            bool eventFilter (QObject *obj, QEvent *event);

        private:
            bool is_key_pressed_ (const KeyState &k);
            void press_key_ (const KeyState &k);
            void release_key_ (const KeyState &k);

            KeyStateVector          state_; 
    };

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

        KeyStateListener *GetKeyState() const;

    public slots:

        void OnAboutToQuit();
        void UpdateFrame();

    private:
        std::auto_ptr <QApplication>        app_;
        std::auto_ptr <QGraphicsView>       view_;
        std::auto_ptr <KeyStateListener>    keylistener_;

        QTimer frame_update_timer_;

        Framework *owner_;
        bool exit_signalled_;

    };

}

#endif
