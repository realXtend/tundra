// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#define QT_QTPROPERTYBROWSER_IMPORT

#include "SceneFwd.h"
#include "CoreTypes.h"
#include "AttributeChangeType.h"

#include <QtTreePropertyBrowser>
#include <QMap>

class QTreeWidget;
class QMenu;
class QMimeData;

class Framework;
class TreeWidgetItemExpandMemory;
struct ComponentGroup;

/// Displays all selected entity-components and their attributes.
/** The ECBrowser will iterate all entity's components and pass them to an ECComponentEditor,
    which is responsible to handling component's attribute editing.

    User can add new editable entities by using AddEntity() and RemoveEntity() methods and the browser will 
    iterate through all the entity's components and pass them to ECComponentEditors.
    ECBrowser has implemented options to add, delete, copy and paste components from the selected entities by
    using a CopyComponent(), DeleteComponent(), PasteComponent() and EditXml() methods.

    User can add attributes to dynamic component by using CreateAttribute() and remove attributes with RemoveAttribute().

    @todo Try to find a way to remove the unnecessary paint events when we are updating the browser parameters.
    @ingroup ECEditorModuleClient. */
class ECBrowser : public QtTreePropertyBrowser
{
    Q_OBJECT

public:
    ECBrowser(Framework *framework, QWidget *parent = 0);
    virtual ~ECBrowser();

    /// Insert new entity to browser.
    /** @note In order to get changes visible, you need to call UpdateBrowser method afterwards.
        @param enity a new entity that we want to edit on the ECEditor. */
    void AddEntity(EntityPtr entity);

    /// Remove edited entity from the browser widget.
    /** @param entity Entity that we want to remove on the ECEditor. */
    void RemoveEntity(EntityPtr entity);

    /// Return list of entities that has added to browser widget. Return empty list if no entities have been added.
    QList<EntityPtr> GetEntities() const;

    /// Sets used item expand memory. Expand memory is used to load and save the expanded items in the tree widget.
    /** @param expandMem Tree widget item expand memory. */
    void SetItemExpandMemory(shared_ptr<TreeWidgetItemExpandMemory> expandMem) { expandMemory_ = expandMem; }

    /// Reads selected components from ComponentGroup and return them as QObjectList.
    QObjectList SelectedComponents() const;

public slots:
    /// Reset browser state to where it was after the browser initialization. Override method from the QtTreePropertyBrowser.
    void clear();

    /// Update editor data and browser ui elements if needed.
    void UpdateBrowser();

    /// Expands or collapses all items in the tree widget, depending on the current state of the widget.
    void ExpandOrCollapseAll();

signals:
    /// User want to open xml editor for that spesific component type.
    /** @param componentType type name of a component. */
    void ShowXmlEditorForComponent(const QString &componentType);

    /// User want to add new component for selected entities.
    void CreateNewComponent();

    /// This method will tell what component and attribute are currently selected from the ECBrowser.
    /** @note If selected QTreeWidget item isn't attribute or it's children, attribute's type/name is marked as empty string.
        @param compType selected item component type name.
        @param compName selected item component name.
        @param attrType selected item attribute type name (Empty if attribute isn't selected).
        @param attrName selected item attribute name (Empty if attribute isn't selected). */
    void SelectionChanged(const QString &compType, const QString &compName, const QString &attrType, const QString &attrName);

protected:
    /// QWidget override.
    void dragEnterEvent(QDragEnterEvent *event);

    /// QWidget override.
    void dropEvent(QDropEvent *event);

    /// QWidget override.
    void dragMoveEvent(QDragMoveEvent *event);

    /// QWidget override.
    void focusInEvent(QFocusEvent *event);

    /// QWidget override.
    /** dropEvent will call this after feching the information that is need from the QDropEvent data. */
    bool dropMimeData(QTreeWidgetItem *item, int index, const QMimeData *data, Qt::DropAction action);

private slots:
    /// User have right clicked the browser and QMenu need to be open to display copy, paste, delete ations etc.
    /** @param pos Mouse click position. */
    void ShowComponentContextMenu(const QPoint &pos);

    /// When QTreeWidget changes it selection a newly selected component need to get highlighted.
    void SelectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);

    /// Called when a new component have been added to a entity.
    /** @param comp a new component that has added into the entity.
        @param type attribute change type. */
    void OnComponentAdded(IComponent* comp, AttributeChange::Type type);

    /// Called when component have been removed from the entity.
    /** @param comp Component that will soon get removed from the entity.
        @param type attribute change type */
    void OnComponentRemoved(IComponent* comp, AttributeChange::Type type);

    /// User has selected xml edit action from a QMenu.
    void OpenComponentXmlEditor();

    /// User has selected copy action from a QMenu.
    void CopyComponent();

    /// User has selected paste action from a QMenu.
    void PasteComponent();

    /// If dynamic component's attributes have changed in some way, we need to find a new suitable group
    /// for the component. If any group cant be found create new and insert component into that group.
    void DynamicComponentChanged();

    /// Removes single attribute of dynamic component from the UI.
    void RemoveAttributeFromDynamicComponent(IAttribute *attr);

    /// Component's name has been changed and we need to remove component from it's previous
    /// ComponentGroup and insert component to another componentgroup.
    /** @param newName component's new name. */
    void OnComponentNameChanged(const QString &newName);

    /// Show dialog, so that user can create a new attribute.
    /// @note Only works with dynamic components.
    void CreateAttribute();

    /// Remove component or attribute based on selected QTreeWidgeItem.
    /** If selected TreeWidgetItem is a root item, then we can assume that we want to remove component.
        But if item has parent set, we can assume that selected item is attribute or it's value is selected. */
    void OnDeleteAction();

    /// Resizes header to contents of the tree widget.
    void ResizeHeaderToContents();

private:
    /// Try to find the right component group for given component.
    /** If right type of component group is found return it's pointer.
        If any suitable componentGroup wasn't found return null pointer.
        @param comp component that we want to find a suitable group. */
    ComponentGroup *FindSuitableGroup(ComponentPtr comp);

    /// Add new component to existing component group if same type of component have been already added to editor,
    /** if component type is not included, create new component group and add it to editor.
        @param comp a new component that we want to add into the enity. */
    void AddNewComponentToGroup(ComponentPtr comp);

    /// Remove component from registered componentgroup. Do nothing if component was not found of any component groups.
    /** @param comp that we want to remove from  the component group. */
    void RemoveComponentFromGroup(ComponentPtr comp);

    /// Remove whole component group object from the browser.
    /** @param componentGroup component group object that we want to remove from the editor. */
    void RemoveComponentGroup(ComponentGroup *componentGroup);

    /// Checks if entity is already added to this editor.
    bool HasEntity(EntityPtr entity) const;

    /// Remove selected attribute item from the dynamic component.
    /** @note This method works only for dynamic components. */
    void DeleteAttribute(QTreeWidgetItem *item);

    /// Remove selected component item from selected entities.
    void DeleteComponent(QTreeWidgetItem *item);

    typedef QMap<QTreeWidgetItem*, ComponentGroup*> TreeItemToComponentGroup;
    TreeItemToComponentGroup itemToComponentGroups_;
    typedef QList<EntityWeakPtr> EntityWeakPtrList;
    EntityWeakPtrList entities_;
    QMenu *menu_;
    QTreeWidget *treeWidget_;
    Framework *framework_;
    weak_ptr<TreeWidgetItemExpandMemory> expandMemory_;
};
