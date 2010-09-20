// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECBrowser_h
#define incl_ECEditorModule_ECBrowser_h

#define QT_QTPROPERTYBROWSER_IMPORT

#include "ForwardDefines.h"
#include "CoreTypes.h"
#include "ComponentGroup.h"

#include <QtTreePropertyBrowser>
#include <map>
#include <set>

class QtTreePropertyBrowser;
class QtGroupPropertyManager;
class QtBrowserItem;
class QTreeWidget;
class QMenu;
class QMimeData;

namespace ECEditor
{
    class ECComponentEditor;
    typedef std::vector<ComponentWeakPtr> ComponentWeakPtrVector;
    typedef std::list<ComponentGroup*> ComponentGroupList;

    //! ECBrowser is a widget that will display all selected entity components and their attributes.
    /*! The ECBrowser will iterate all entity's components and pass them to a ECComponentEditor switch is responsible to handling component's attribute editing.
     *  User can add new editable enitites by using AddEntity and RemoveEntity methods and the browser will iterate trhought all the entitys components and pass them to ECComponentEditors.
     *  ECBrowser has implement options to add, delete, copy and paste components from the selected entities by using a CopyComponent, DeleteComponent, PasteComponent and EditXml mehtods.
     *  User can add and remove attributes from the dynamic component aswell. CreateAttribute method can be used to create new attributes to a dynamic component and RemoveAttribute is used
     *  to delete them from the dynamic component.
     *  \todo Try to find a way to remove the unecessary paint events when we are updating the browser parameters.
     *  \ingroup ECEditorModuleClient.
     */
    class ECBrowser: public QtTreePropertyBrowser
    {
        Q_OBJECT
    public:
        ECBrowser(Foundation::Framework *framework, QWidget *parent = 0);
        virtual ~ECBrowser();

        //! Insert new entity to browser and add it's components to the browser.
        //! Note! to get changes visible you need to call UpdateBrowser method.
        /*! @param enity a new entity that we want to edit on the ECEditor.
         */
        void AddNewEntity(Scene::Entity *entity);
        //! Remove edited entity from the browser widget.
        /*! @param entity Entity that we want to remove on the ECEditor.
         */
        void RemoveEntity(Scene::Entity *entity);

    public slots:
        //! Reset browser state to where it was after the browser initialization. Override method from the QtTreePropertyBrowser.
        void clear();
        //! Update editor data and browser ui elements if needed.
        void UpdateBrowser();

    signals:
        //! User want to open xml editor for that spesific component type.
        /*! @param componentType type name of a component.
         */
        void ShowXmlEditorForComponent(const std::string &componentType);
        //! User want to add new component for selected entities.
        void CreateNewComponent();
        //! Emitted when component is selected from the browser widget.
        /*! @param component Pointer to a component that has just been selected.
         */
        void ComponentSelected(IComponent *component);

    protected:
        //! Override from QWidget.
        void dragEnterEvent(QDragEnterEvent *event);
        //! Override from QWidget.
        void dropEvent(QDropEvent *event);
        //! Override from QWidget.
        void dragMoveEvent(QDragMoveEvent *event);
        //! dropEvent will call this after feching the information that is need from the QDropEvent data.
        bool dropMimeData(QTreeWidgetItem *item, int index, const QMimeData *data, Qt::DropAction action);

    private:
        //! Initialize browser widget and create all connections for different QObjects.
        void InitBrowser();
        //! Try to find the right component group for spesific component type. if found return it's position on the list as in iterator format.
        //! If any component group wasn't found return .end() iterator value.
        /*! @param comp component that we want to find in some of the component group.
         */
        ComponentGroupList::iterator FindSuitableGroup(const IComponent &comp);
        //! Try to find component group for spesific QTreeWidgetItem.
        /*! @param item QTreeWidgetItem that we want to use to find a right component group.
         */
        ComponentGroupList::iterator FindSuitableGroup(const QTreeWidgetItem &item);
        //! Add new component to existing component group if same type of component have been already added to editor,
        /*! if component type is not included, create new component group and add it to editor.
         *  @param comp a new component that we want to add into the enity.
         */
        void AddNewComponentToGroup(ComponentInterfacePtr comp);
        //! Remove component from registered componentgroup. Do nothing if component was not found of any component groups.
        /*! @param comp that we want to remove from  the component group.
         */
        void RemoveComponentFromGroup(IComponent *comp);
        //! Remove whole component group object from the browser.
        /*! componentGroup component group object that we want to remove from the editor.
         */
        void RemoveComponentGroup(ComponentGroup *componentGroup);

    private slots:
        //! User have right clicked the browser and QMenu need to be open to display copy, paste, delete ations etc.
        /*! @param pos Mouse click position.
         */
        void ShowComponentContextMenu(const QPoint &pos);
        //! QTreeWidget has changed it's focus and we need to highlight new entities from the editor window.
        void SelectionChanged();
        //! a new component have been added to entity.
        /*! @param entity Entity that owns the component
         *  @param comp a new component that has added into the entity.
         */
        void NewComponentAdded(Scene::Entity* entity, IComponent* comp);
        //! Component have been removed from the entity.
        /*! @param entity Entity that owns the component
         *  @param comp Component that is planned to be removed from the entity.
         */
        void ComponentRemoved(Scene::Entity* entity, IComponent* comp);
        //! User has selected xml edit action from a QMenu.
        void OpenComponentXmlEditor();
        //! User has selected copy action from a QMenu.
        void CopyComponent();
        //! User has selected paste action from a QMenu.
        void PasteComponent();
        //! User has selected delete action from a QMenu.
        void DeleteComponent();
        //! New dynamic component attribute has been added.
        /*! @todo When many attributes has been added/removed from the editor this method is called multiple times and each time this method seems to reinitialize the component editor.
         * This will consume too much time and should be fixed so that coponent editor will be initialized only once when all the dynamic component's attributes have been added.
         */
        void DynamicComponentChanged(const QString &name);
        //! Component's name has been changed and we need to remove component from it's previous ComponentGroup and create/add component to another componentgroup.
        /*! @param newName component's new name.
         */
        void ComponentNameChanged(const std::string &newName);
        //! Show dialog, so that user can create a new attribute.
        void CreateAttribute();
        //! Remove selected attribute from the dynamic component.
        void RemoveAttribute();

    private:
        ComponentGroupList componentGroups_;
        typedef std::set<Scene::Entity *> EntitySet;
        EntitySet selectedEntities_;
        QMenu *menu_;
        QTreeWidget *treeWidget_;
        Foundation::Framework *framework_;
    };
}

#endif