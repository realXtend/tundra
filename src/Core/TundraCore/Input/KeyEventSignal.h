// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include "TundraCoreApi.h"
#include "InputFwd.h"
#include <QObject>
#include <QKeySequence>

/// A signal object for input events to a specific key on the keyboard.
class TUNDRACORE_API KeyEventSignal : public QObject
{
    Q_OBJECT

signals:
    void SequencePressed(KeyEvent *key);
    void SequenceReleased(KeyEvent *key);
//    void SequenceDown(KeyEvent *key);

public:
    explicit KeyEventSignal(QKeySequence sequence) : keySequence(sequence) {}

    /// This is the key sequence that this key signal is triggered for.
    const QKeySequence keySequence;

    void OnKeyPressed(KeyEvent *key) { emit SequencePressed(key); }
//    void OnKeyDown(KeyEvent *key) { emit SequenceDown(key); }
    void OnKeyReleased(KeyEvent *key) { emit SequenceReleased(key); }
};
Q_DECLARE_METATYPE(KeyEventSignal*);
