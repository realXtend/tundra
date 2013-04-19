/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   UndoManager.cpp
    @brief  UndoManager class which manages the undo stack and provides drop-down menus with the most recent undo/redo commands.*/

#include "StableHeaders.h"
#include "UndoManager.h"
#include "EntityIdChangeTracker.h"

#include <QUndoCommand>
#include <QAction>

#include "MemoryLeakCheck.h"

UndoManager::UndoManager(const ScenePtr &scene, QWidget *parent)
{
    undoStack_ = new QUndoStack();

    undoMenu_ = new QMenu();
    redoMenu_ = new QMenu();
    undoViewAction_ = new QAction("View all", 0);

    tracker_ = new EntityIdChangeTracker(scene);

    undoView_ = new QUndoView(undoStack_, parent);
    undoView_->setWindowFlags(Qt::Tool);
    undoView_->setWindowTitle("Editor - Undo stack");

    connect(undoStack_, SIGNAL(indexChanged(int)), this, SLOT(OnIndexChanged(int)));
    connect(undoStack_, SIGNAL(canUndoChanged(bool)), this, SLOT(OnCanUndoChanged(bool)));
    connect(undoStack_, SIGNAL(canRedoChanged(bool)), this, SLOT(OnCanRedoChanged(bool)));
    connect(undoMenu_, SIGNAL(triggered(QAction *)), this, SLOT(OnActionTriggered(QAction *)));
    connect(redoMenu_, SIGNAL(triggered(QAction *)), this, SLOT(OnActionTriggered(QAction *)));
    connect(undoViewAction_, SIGNAL(triggered(bool)), undoView_, SLOT(show()));
}

UndoManager::~UndoManager()
{
    if (undoStack_)
    {
        disconnect(undoStack_, SIGNAL(indexChanged(int)), this, SLOT(OnIndexChanged(int)));
        disconnect(undoStack_, SIGNAL(canUndoChanged(bool)), this, SLOT(OnCanUndoChanged(bool)));
        disconnect(undoStack_, SIGNAL(canRedoChanged(bool)), this, SLOT(OnCanRedoChanged(bool)));
    }
    SAFE_DELETE(undoStack_);

    // Parented, don't delete
    if (undoView_ && undoView_->parent())
        undoView_ = 0;
    else
        SAFE_DELETE(undoView_);
    
    SAFE_DELETE(undoMenu_);
    SAFE_DELETE(redoMenu_);
    SAFE_DELETE(undoViewAction_);
    SAFE_DELETE(tracker_);
}

QMenu * UndoManager::UndoMenu() const
{
    return undoMenu_;
}

QMenu * UndoManager::RedoMenu() const
{
    return redoMenu_;
}

EntityIdChangeTracker * UndoManager::Tracker() const
{
    return tracker_;
}

void UndoManager::OnCanUndoChanged(bool canUndo)
{
    if (!canUndo)
        undoMenu_->clear();

    emit CanUndoChanged(canUndo);
}

void UndoManager::OnCanRedoChanged(bool canRedo)
{
    if (!canRedo)
        redoMenu_->clear();

    emit CanRedoChanged(canRedo);
}

void UndoManager::Undo()
{
    undoStack_->undo();
}

void UndoManager::Redo()
{
    undoStack_->redo();
}

void UndoManager::Clear()
{
    undoStack_->clear();
    tracker_->Clear();
}

void UndoManager::OnIndexChanged(int idx)
{
    undoMenu_->clear();
    redoMenu_->clear();

    int maxActions = 5;

    for (UndoRedoActionList::reverse_iterator i = actions_.rbegin(); i != actions_.rend(); ++i)
    {
        if (idx > ((*i)->property("index").toInt()))
        {
            maxActions--;
            (*i)->setProperty("actionType", "undo");
            undoMenu_->addAction(*i);
        }

        if (maxActions == 0)
            break;
    }

    maxActions = 5;
    for (UndoRedoActionList::iterator i = actions_.begin(); i != actions_.end(); ++i)
    {
        if (idx <= ((*i)->property("index").toInt()))
        {
            maxActions--;
            (*i)->setProperty("actionType", "redo");
            redoMenu_->addAction(*i);
        }

        if (maxActions == 0)
            break;
    }

    undoViewAction_->setText(QString("View all %1 item(s)").arg(undoStack_->count()));

    undoMenu_->addSeparator();
    undoMenu_->addAction(undoViewAction_);
    redoMenu_->addSeparator();
    redoMenu_->addAction(undoViewAction_);
}

void UndoManager::OnActionTriggered(QAction * action)
{
    int index = action->property("index").toInt();

    if (action->property("actionType").toString() == "undo")
        undoStack_->setIndex(index);
    else
        undoStack_->setIndex(index + 1);
}

void UndoManager::Push(QUndoCommand * command)
{
    actions_.resize(undoStack_->index());

    QAction * action = new QAction(command->text(), 0);
    action->setProperty("index", undoStack_->index());

    actions_.push_back(action);
    undoStack_->push(command);
}
