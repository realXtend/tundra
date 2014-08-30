/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   UndoManager.h
    @brief  UndoManager class which manages the undo stack and provides drop-down menus with the most recent undo/redo commands.*/

#pragma once

#include "ECEditorModuleApi.h"
#include "SceneFwd.h"

#include <QObject>
#include <QPointer>

class EntityIdChangeTracker;

class QMenu;
class QUndoStack;
class QUndoCommand;
class QUndoView;
class QAction;

/// Manages the undo stack and provides drop-down menus with the most recent undo/redo commands.
class ECEDITOR_MODULE_API UndoManager : public QObject
{
    Q_OBJECT

public:
    /// Constructor
    /** @note The actual UndoManager QObject is not parented to anything, the widgets can be optionally parented.
        @param Scene of which entities we're tracking.
        @param Parent for the internal QUndoView. this top level window is shown when the full undo/redo is viewed.
               If you use parenting, set a widget that when closed should also close the view.
        @param Parent for the undo popup menu.
        @param Parent for the redo popup menu. */
    UndoManager(const ScenePtr &scene, QWidget *parentWidget = 0, QWidget *undoMenuParent = 0, QWidget *redoMenuParent = 0);

    /// Dtor.
    ~UndoManager();

    /// Pushes new command to the undo stack
    void Push(QUndoCommand * command);

    /// The menu containing actions that can be undone
    /** @note Prefer not to store the raw ptr. The menu may be destroyed before
        this UndoManager is destroyed due to QWidget parenting. */
    QMenu *UndoMenu() const;

    /// The menu containing actions that can be redone
    /** @note Prefer not to store the raw ptr. The menu may be destroyed before
        this UndoManager is destroyed due to QWidget parenting. */
    QMenu *RedoMenu() const;
    
    /// The top level list view widget that can be shown from the undo/redo menus.
    /** @note Prefer not to store the raw ptr. The undo view may be destroyed before
        this UndoManager is destroyed due to QWidget parenting. */
    QUndoView *UndoView() const;

    /// Returns a pointer to the entity ID change tracker
    EntityIdChangeTracker *Tracker() const;

    /// Return command by type. See UndoCommands.h for the Tundra types.
    template <typename T>
    QList<const T*> Commands() const
    {
        QList<const T*> result;
        if (!undoStack_)
            return result;
        for(int i=0, len=undoStack_->count(); i<len; ++i)
        {
            const T *cmd = dynamic_cast<const T*>(undoStack_->command(i));
            if (cmd)
                result << cmd;
        }
        return result;
    }

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

    /// CanRedoChanged is emitted when redo is possible, i.e. a command has been undo-ed
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
    // Initialize user interface, called from various types of ctors.
    void Initialize(QWidget *parent = 0, QWidget *undoMenuParent = 0, QWidget *redoMenuParent = 0);

    std::list<QAction*> actions_;       ///< Action list

    // Always unparented.
    QAction *undoViewAction_;           ///< Undo view action
    QUndoStack *undoStack_;             ///< Undo stack
    EntityIdChangeTracker * tracker_;   ///< Entity ID change tracker

    // Optionally parented, danger of dangling ptrs.
    QPointer<QMenu> undoMenu_;          ///< Undo menu
    QPointer<QMenu> redoMenu_;          ///< Redo menu
    QPointer<QUndoView> undoView_;      ///< Undo view    
};
