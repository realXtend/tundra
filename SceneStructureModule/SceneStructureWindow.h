/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureWindow.h
 *  @brief  
 */

#ifndef incl_SceneStructureModule_SceneStructureWindow_h
#define incl_SceneStructureModule_SceneStructureWindow_h


#include "ForwardDefines.h"

#include <QWidget>
#include <QMap>

class QModelIndex;

class SceneTreeWidget;

///
/**
*/
class SceneStructureWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window and populates tree view with entities.
    /** @param fw Framework.
    */
    explicit SceneStructureWindow(Foundation::Framework *fw);

    /// Destructor.
    ~SceneStructureWindow();

    /// Sets new scene to be shown in the tree view.
    /** @param s Scene.
    */
    void SetScene(const Scene::ScenePtr &s);

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

private slots:
    ///
    /** @param entity
    */
    void AddEntity(Scene::Entity* entity);

    ///
    /** @param entity
    */
    void RemoveEntity(Scene::Entity* entity);

    void EditEntity(const QModelIndex &index);
};

#endif
