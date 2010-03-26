// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Foundation_KeyBindings_h
#define incl_Foundation_KeyBindings_h

#include "CoreTypes.h"

#include <QKeySequence>
#include <QStringList>

namespace Foundation
{
    struct EventPair
    {
        EventPair(event_id_t enter_event_id, event_id_t leave_event_id = 0);
        EventPair(std::pair<int,int> event_ids);
        bool operator==(EventPair compare_pair);

        event_id_t enter_id;
        event_id_t leave_id;
    };

    struct Binding
    {
        Binding(QKeySequence key_sequence, EventPair event_id_pair);
        bool operator==(Binding binding);

        QKeySequence sequence;
        EventPair event_ids;
        QString config_key;
    };

    typedef std::list<Binding> KeyBindingList;

    class KeyBindings
    {
    
    public:
        KeyBindings();
        void BindKey(Binding binding);
        void BindKey(QKeySequence sequence, std::pair<int,int> event_ids);

        KeyBindingList GetBindings();
        std::list<Binding> GetBindings(EventPair event_pair);
        std::list<Binding> GetBindings(QString event_config_key);
        std::pair<int,int> GetEventPair(QKeySequence sequence);
        QStringList GetConfigKeys();

        QString NameForEvent(int event_id);
        std::pair<int,int> EventPairForName(QString name);
        
    private:
        KeyBindingList bindings_;
        QStringList config_keys_;
    };
}

#endif