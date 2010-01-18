
#include "StableHeaders.h"
#include "State.h"

namespace Foundation
{
    //=========================================================================
    //
    State::State (QString name, StateMap &r, QState *parent) 
        : QState (parent), active (false), registry (r)
    { 
        setObjectName (name); 
        registry.insert (std::make_pair (name, this));
    }

    State::State (QString name, StateMap &r, QState::ChildMode mode, QState *parent) 
        : QState (mode, parent), active (false), registry (r)
    { 
        setObjectName (name); 
        registry.insert (std::make_pair (name, this));
    }

    State::~State ()
    {
        StateMap::iterator i = registry.find (objectName());
        StateMap::iterator e = registry.end ();
        if (i != e) registry.erase (i);
    }

    void State::onEntry (QEvent *e)
    {
        //std::cout << "State::onEntry: " << qPrintable (objectName()) << std::endl;
        active = true;
    }

    void State::onExit (QEvent *e)
    {
        //std::cout << "State::onExit: " << qPrintable (objectName()) << std::endl;
        active = false;
    }
}
