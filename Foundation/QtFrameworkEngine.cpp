// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "QOgreUIView.h"

#include <sstream>
#include <string>

#include <QApplication>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QFrame>

#ifndef Q_WS_WIN
#include <QX11Info>
#endif

#include "Framework.h"

#include "QtFrameworkEngine.h"

namespace Foundation
{

    QtFrameworkEngine::QtFrameworkEngine(Framework *owner, int &argc, char** argv) : 
        owner_(owner)
    {
        app_.reset (new QApplication(argc, argv));
        keylistener_.reset (new KeyStateListener);
    }

    QtFrameworkEngine::~QtFrameworkEngine()
    {
        keylistener_.reset(0);
        view_.reset(0);
        app_.reset(0);
    }

    QApplication *QtFrameworkEngine::GetQApplication() const
    {
        return app_.get();
    }

    QGraphicsView *QtFrameworkEngine::GetUIView() const
    {
        return view_.get();
    }

    void QtFrameworkEngine::SetUIView(std::auto_ptr <QGraphicsView> view)
    {
        view_ = view;
        //view_-> installEventFilter (keylistener_.get());
    }

    KeyStateListener *QtFrameworkEngine::GetKeyState () const
    {
        return keylistener_.get();
    }

    void QtFrameworkEngine::UpdateFrame()
    {
        assert(owner_);
        owner_->ProcessOneFrame();
        frame_update_timer_.start(0);
    }

    void QtFrameworkEngine::Go()
    {
        assert(app_.get());

        QObject::connect(app_.get(), SIGNAL(aboutToQuit()), this, SLOT(OnAboutToQuit()));
        QObject::connect(&frame_update_timer_, SIGNAL(timeout()), this, SLOT(UpdateFrame()));
        
        // with single shot and start(0) processor usage = 90-98% no matter the size of window
        // with start(20) its <50% all the time
        frame_update_timer_.setSingleShot(true);
        frame_update_timer_.start(0); 

        app_->exec();
    }

    void QtFrameworkEngine::OnAboutToQuit()
    {
        assert(owner_);
        if (owner_)
            owner_->Exit();
    }

    void QtFrameworkEngine::SendQAppQuitMessage()
    {
        assert(app_.get());
        if (app_.get())
            app_->quit();
    }

    //=============================================================================
    // Keyboard State

    KeyState::KeyState () : 
        key (0), 
        modifiers (0) 
    {
    }

    KeyState::KeyState (int k, unsigned int m) : 
        key (k), modifiers (m)
    {
    }

    KeyState::KeyState (const KeyState &c) : 
        key (c.key), 
        modifiers (c.modifiers) 
    {
    }

    KeyState::KeyState (const QKeyEvent *e) : 
        key (e-> key()), 
        modifiers (e-> modifiers()) 
    {
    }

    KeyState &KeyState::operator= (const KeyState &rhs) 
    { 
        key = rhs.key; 
        modifiers = rhs.modifiers; 
        return *this; 
    }

    bool KeyState::operator== (const KeyState &rhs) const
    {
        return (key == rhs.key);
    }

    bool KeyState::operator== (int k) const
    {
        return (key == k);
    }

    //=============================================================================
    // Current State of the Keyboard

    KeyStateListener::KeyStateListener ()
    {
        state_.reserve (10); // we presume users only have 10 fingers
    }

    char KeyStateListener::KeyToAscii (const KeyState &k)
    {
        if ((k.key > 0x1f) && (k.key < 0x7f))
        { // ASCII
            char c (k.key);
            if ((c > 0x40) && (c < 0x5b)) // upper
                if (!(k.modifiers & 0x02000000))
                    c |= 0x20; // to lower
            return c;
        }
        else return 0;
    }

    KeyState KeyStateListener::AsciiToKey (char c)
    {
        KeyState k;
        if ((c > 0x1f) && (c < 0x7f))
        { // printable characters
            if ((c > 0x40) && (c < 0x5b)) // upper
                k.modifiers |= 0x02000000;

            if ((c > 0x61) && (c < 0x7b)) // lower
                c &= 0xdf; // to upper
        }
        return k;
    }

    const std::vector<KeyState> &KeyStateListener::GetState () 
    { 
        return state_; 
    }

    const std::vector<char> KeyStateListener::GetCharState () 
    { 
        std::vector <char> charstate;

        std::vector <KeyState>::iterator i (state_.begin());
        for (; i != state_.end(); ++i)
            charstate.push_back (KeyToAscii (*i));

        return charstate; 
    }

    bool KeyStateListener::IsKeyDown (const KeyState &key) 
    { 
        return is_key_pressed_ (key); 
    }

    bool KeyStateListener::IsKeyDown (int key, unsigned int modifiers) 
    { 
        return is_key_pressed_ (KeyState (key, modifiers)); 
    }

    bool KeyStateListener::eventFilter (QObject *obj, QEvent *event)
    {
        //std::cout << "event (Qt): " << event->type() << std::endl;
        switch (event-> type())
        {
            case QEvent::KeyPress:
                press_key_ (KeyState (static_cast <QKeyEvent *> (event)));
                return false;

            case QEvent::KeyRelease:
                release_key_ (KeyState (static_cast <QKeyEvent *> (event)));
                return false;

            default:
                return QObject::eventFilter (obj, event);
        }
    }

    bool KeyStateListener::is_key_pressed_ (const KeyState &k)
    {
        return (std::find (state_.begin(), state_.end(), k) != state_.end());
    }

    void KeyStateListener::press_key_ (const KeyState &k)
    { 
        //std::cout << "key pressed: " << KeyToAscii (k) << std::endl;
        state_.push_back (k); 
    }

    void KeyStateListener::release_key_ (const KeyState &k)
    { 
        state_.erase (std::find (state_.begin(), state_.end(), k)); 
    }

}
