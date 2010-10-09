/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureWindow.h
 *  @brief  Window with tree view of contents of scene.
 *
 *          Detailed desc here.
 */

#ifndef incl_SceneStructureModule_SceneStructureWindow_h
#define incl_SceneStructureModule_SceneStructureWindow_h


#include "ForwardDefines.h"

#include <QWidget>
#include <QMap>

class QModelIndex;

class SceneTreeWidget;

/// Window with tree view of contents of scene.
/** Detailed desc here.
*/
class SceneStructureWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param fw Framework.
    */
    explicit SceneStructureWindow(Foundation::Framework *fw);

    /// Destructor.
    ~SceneStructureWindow();

    /// Sets new scene to be shown in the tree view.
    /** Populates tree view with entities.
        If scene is set to 0, the tree view is cleared and signal connections are disconnected.
        @param s Scene.
    */
    void SetScene(const Scene::ScenePtr &s);

public slots:
    /// Sets do we want to show components in the tree view.
    /** @param show Visibility of components in the tree view.
    */
    void ShowComponents(bool show);

protected:
    /// QWidget override.
    void changeEvent(QEvent* e);

private:
    /// Populates tree widget with all entities.
    void Populate();

    /// Clears tree widget.
    void Clear();

    /// Framework.
    Foundation::Framework *framework;

    /// Scene which we are showing the in tree widget currently.
    Scene::SceneWeakPtr scene;

    /// Scene tree widget.
    SceneTreeWidget *treeWidget;

    /// Do we show components also in the tree view.
    bool showComponents;

private slots:
    /// Adds the entity to the tree widget.
    /** @param entity Entity to be added.
    */
    void AddEntity(Scene::Entity* entity);

    /// Removes entity from the tree widget.
    /** @param entity Entity to be removed.
    */
    void RemoveEntity(Scene::Entity* entity);

    /// Adds the entity to the tree widget.
    /** @param entity Altered entity.
        @param comp Component which was added.
    */
    void AddComponent(Scene::Entity* entity, IComponent* comp);

    /// Removes entity from the tree widget.
    /** @param entity Aletred entity.
        @param comp Component which was removed.
    */
    void RemoveComponent(Scene::Entity* entity, IComponent* comp);
};

#endif
