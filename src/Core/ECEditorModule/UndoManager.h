/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   UndoManager.h
    @brief  UndoManager class which manages the undo stack and provides drop-down menus with the most recent undo/redo commands 
*/

#pragma once

#include <QObject>

class Scene;

class QMenu;
class QUndoStack;
class QUndoCommand;
class QUndoView;
class QAction;
class EntityIdChangeTracker;

class UndoManager : public QObject
{
    Q_OBJECT

public:
    /// Constructor
    /* @param scene A raw pointer to the main camera scene
       @param parent This object's parent (optional) */
    UndoManager(Scene * scene, QObject * parent);

    /// Destructor;
    ~UndoManager();

    /// Pushes new command to the undo stack
    /* @param command The command to be pushed. */
    void Push(QUndoCommand * command);

    /// Returns a pointer to the undo menu containing actions that can be undo-ed
    /* @returns Undo menu */
    QMenu * UndoMenu() const;

    /// Returns a pointer to the redo menu containing actions that can be redo-ed
    /* @returns Redo menu */
    QMenu * RedoMenu() const;

    /// Returns a pointer to the entity ID change tracker
    /* @returns Entity ID change tracker */
    EntityIdChangeTracker * GetTracker();

public slots:
    /// Calls the undo stack's undo() method
    void Undo();
    /// Calls the undo stack's redo() method
    void Redo();
    /// Clears the undo stack and the actions
    void Clear();

signals:
    /// CanUndoChanged is emitted when undo is possible, i.e. a command has been pushed onto the stack
    /* @param canUndo New undo state */
    void CanUndoChanged(bool canUndo);

    /// CanRedoChanged is emitted when redo is possible, i.e. a command has ben undo-ed
    /* @param canRedo New redo state */
    void CanRedoChanged(bool canRedo);

private slots:
    /// Handles undo stack's indexChanged signal and repopulates the menu i.e. splits the actions among the two menus
    /* @param idx The current index */
    void OnIndexChanged(int idx);

    /// Handles undo stack's canUndoChanged and emits CanUndoChanged further. Also clears the undo menu if canUndo is false
    /* @param canUndo New undo state */
    void OnCanUndoChanged(bool canUndo);

    /// Handles undo stack's canRedoChanged and emits CanRedoChanged further. Also clears the redo menu if canRedo is false
    /* @param canRedo New redo state */
    void OnCanRedoChanged(bool canRedo);

    /// Sets undo stack's index accordingly to the action that is triggered
    /* @param action The action that is triggered */
    void OnActionTriggered(QAction * action);

private:
    QMenu * undoMenu_; ///< Undo menu
    QMenu * redoMenu_; ///< Redo menu
    typedef std::list<QAction *> UndoRedoActionList; ///< Action list typedef
    UndoRedoActionList actions_; ///< Action list
    QAction * undoViewAction_; ///< Undo view action
    EntityIdChangeTracker * tracker_; ///< Entity ID change tracker

    QUndoView * undoView_; ///< Undo view
    QUndoStack * undoStack_; ///< Undo stack
};
