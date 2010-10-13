/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneStructureWindow.h
 *  @brief  Window with tree view showing every entity in a scene.
 */

#ifndef incl_SceneStructureModule_SceneStructureWindow_h
#define incl_SceneStructureModule_SceneStructureWindow_h


#include "ForwardDefines.h"

#include <QWidget>
#include <QMap>

class QTreeWidgetItem;

class SceneTreeWidget;
class EntityItem;
class ComponentItem;

/// Window with tree view showing every entity in a scene.
/** This class will only handle adding and removing of entities and components and updating
    their names. The SceneTreeWidget implements most of the functionlity.
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

    /// Sets do we want to show asset references in the tree view.
    /** @param show Visibility of asset references in the tree view.
    */
    void ShowAssetReferences(bool show);

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

    /// Do we show asset references also in the tree view.
    bool showAssets;

private slots:
    /// Adds the entity to the tree widget.
    /** @param entity Entity to be added.
    */
    void AddEntity(Scene::Entity *entity);

    /// Removes entity from the tree widget.
    /** @param entity Entity to be removed.
    */
    void RemoveEntity(Scene::Entity *entity);

    /// Adds the entity to the tree widget.
    /** @param entity Altered entity.
        @param comp Component which was added.
    */
    void AddComponent(Scene::Entity *entity, IComponent *comp);

    /// This is an overload function.
    /** @param parentItem
        @param comp Component which was added.
    */
    void AddComponent(EntityItem *parentItem, IComponent *comp);

    /// Removes entity from the tree widget.
    /** @param entity Aletred entity.
        @param comp Component which was removed.
    */
    void RemoveComponent(Scene::Entity *entity, IComponent *comp);

    /// Adds asset reference to the tree widget.
    /** @param parentItem Parent item, can be entity or component item.
        @param attr AssetReference attribute.
    */
    void AddAssetReference(QTreeWidgetItem *parentItem, IAttribute *attr);

    /// Removes asset reference from the tree widget.
    /** @param 
        @param attr AssetReference attribute.
    */
    void RemoveAssetReference(IAttribute *attr);

    /// Updates entity's name in the tree widget if entity's EC_Name component's "name" attribute has changed.
    /** EC_Name component's OnAttributeChanged() signal is connected to this slot.
        @param attr EC_Name's attribute which was changed.
    */
    void UpdateEntityName(IAttribute *attr);

    /// Updates component's name in the tree widget if components name has changed.
    /** @param oldName Old component name.
        @param newName New component name.
    */
    void UpdateComponentName(const QString &oldName, const QString &newName);
};

#endif
