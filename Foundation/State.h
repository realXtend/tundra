// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_State_h
#define incl_Foundation_State_h

#include <map>
#include <QState>
#include <QString>

namespace Foundation
{
    struct State;

    //! Records a manifest of known Named States
    typedef std::map <QString, State *> StateMap;

    //! Named State structure (based on QState) for building and querying state machines
    struct State : public QState
    {
        State (QString name, StateMap &registry, QState *parent = 0);
        State (QString name, StateMap &registry, QState::ChildMode mode, QState *parent = 0);
        virtual ~State ();

        virtual void onEntry (QEvent *e);
        virtual void onExit (QEvent *e);
                
        bool active;

        StateMap &registry;
    };
}

#endif
