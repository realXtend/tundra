// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_AttributePropertyBrowser_h
#define incl_ECEditorModule_AttributePropertyBrowser_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include <QtTreePropertyBrowser>
#include <map>
#include <set>
#include "CoreTypes.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class QtTreePropertyBrowser;
class QtGroupPropertyManager;
class QtBrowserItem;
class QTreeWidget;
class QMenu;

namespace Foundation
{
    class ComponentInterface;
    typedef boost::shared_ptr<ComponentInterface> ComponentInterfacePtr;
    typedef boost::weak_ptr<ComponentInterface> ComponentWeakPtr;
}

namespace Scene
{
    class Entity;
    typedef boost::shared_ptr<Entity> EntityPtr;
    typedef boost::weak_ptr<Entity> EntityWeakPtr;
}

namespace ECEditor
{
    class ECComponentEditor;
    typedef std::vector<Foundation::ComponentWeakPtr> ComponentWeakPtrVector;

    class AttributeBrowser : public QtTreePropertyBrowser
    {
        Q_OBJECT
    public:
        AttributeBrowser(QWidget *parent = 0);
        ~AttributeBrowser();

    public slots:
        //! Check component's type name and if there isn't any ComponentEditor created for that spesific type name create a new istance
        //! of that editor. If editor is already created add new compoent to ComponentEditor that will handle the ui updates (Multiedit).
        //! @param newComponent new component that we want to add to edit.
        void CreateOrAddNewComponentToEditor(Foundation::ComponentInterfacePtr newComponent);

        //! Add new component to editor browser.
        void AddNewComponent(Foundation::ComponentInterfacePtr component);

        //! Remove component from editor browser.
        void RemoveComponent(Foundation::ComponentInterfacePtr component);

        //! Iterate all the entity's components and add them into the editor window.
        //! @param entity New entity pointer.
        void AddNewEntity(Scene::EntityPtr entity);

        //! Remove all entity's components from the browser.
        void RemoveEntity(Scene::Entity *entity);

        //! add entity components that we want to edit with attribute browser.
        //! @param entityComponents list of components that we want to edit.
        void AddEntityComponents(std::vector<Foundation::ComponentInterfacePtr> entityComponents);

        //! Reset Attribute editor's state to where it was while it was first time intialized.
        void clear();

    signals:
        //! Some of this browser's attribute has been changed by user.
        //! @param attributeName attribute name that has been changed.
        void AttributesChanged(const std::string &attributeName);

        //! User want to open xml editor for that spesific component type.
        void ShowXmlEditorForComponent(const std::string &componentType);

        //! User want to add new component for selected entities.
        void CreateNewComponent();

        //! User wants to remove compoent for selected entities.
        void DeleteComponent(const std::string &componentType);

        //! Copy components attribute values in clipboard.
        void CopyComponent(Foundation::ComponentInterfacePtr component);

        //! If clipboard contain attribute valeus insert them into the selected components.
        void PasteComponent(Foundation::ComponentInterfacePtr component);

    private slots:
        //! Listens when the entity has added a new component. Add new compoent to editor and update browser ui.
        //! @param entity entity that owns the removed component.
        //! @param comp entity component that is removed from the entity.
        void NewEntityComponentAdded(Scene::Entity* entity, Foundation::ComponentInterface* comp);

        //! Listens when the entity has removed one of it's components. Remove component from the editor and
        //! update browser ui.
        //! @param entity entity that owns the removed component.
        //! @param comp entity component that is removed from the entity.
        void EntityComponentRemoved(Scene::Entity* entity, Foundation::ComponentInterface* comp);

        //! Shows context menu for components.
        //! @param pos Mouse position of right-click event.
        void ShowComponentContextMenu(const QPoint &pos);

        //! Open xml editor.
        void OpenComponentXmlEditor();

        //! User has selected copy action from a QMenu.
        void CopyComponent();

        //! User has selected paste action from a QMenu.
        void PasteComponent();

        //! Delete selected component from scene and editor.
        void DeleteComponent();

    private:
        void InitializeEditor();

        typedef std::map<std::string, ECComponentEditor *> ComponentEditorMap;
        ComponentEditorMap componentEditors_;
        typedef std::map<QtProperty *, ComponentWeakPtrVector> PropertyToComponentsMap;
        PropertyToComponentsMap componentsBrowserItemMap_;
        typedef std::set<entity_id_t> EntityIDSet;
        EntityIDSet selectedEntities_;
        QMenu *menu_;
        QTreeWidget *treeWidget_;
    };
}

#endif