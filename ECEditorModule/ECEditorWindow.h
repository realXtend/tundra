/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.h
 *  @brief  Entity-component editor window.
 */

#ifndef incl_ECEditorModule_ECEditorWindow_h
#define incl_ECEditorModule_ECEditorWindow_h

#if defined (_WINDOWS)
#if defined(ECEDITOR_MODULE_EXPORTS)
#define ECEDITOR_MODULE_API __declspec(dllexport)
#else
#define ECEDITOR_MODULE_API __declspec(dllimport)
#endif
#else
#define ECEDITOR_MODULE_API
#endif

#include "ForwardDefines.h"
#include "CoreTypes.h"
//#include <set>
#include <QMap>
#include <QSet>

#include <QWidget>

extern std::vector<std::string> AttributeTypenames;

QT_BEGIN_NAMESPACE;
class QPushButton;
class QListWidget;
class QTreeWidget;
class QPoint;
QT_END_NAMESPACE;

struct EntityComponentSelection
{
    Scene::EntityPtr entity;
    std::vector<ComponentPtr> components;
};

namespace ECEditor
{
    class ECBrowser;
    class AddComponentDialog;

    /// Entity-component editor window.
    /** @todo add description.
        \ingroup ECEditorModuleClient.
    */
    class ECEDITOR_MODULE_API ECEditorWindow : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor
        /** @param framework Framework.
        */
        explicit ECEditorWindow(Foundation::Framework* framework);

        /// Destructor.
        ~ECEditorWindow();

        /// Adds new entity to the entity list.
        /** @param id Entity ID.
        */
        void AddEntity(entity_id_t id);

        /// Removes entity from the entity list.
        /** @param id Entity ID.
        */
        void RemoveEntity(entity_id_t id);

        /// Sets which entities are selected in the editor window.
        /** @param ids List of entity ID's.
        */
        void SetSelectedEntities(const QList<entity_id_t> ids);

        /// Clears entity list.
        void ClearEntities();

    public slots:
        /// Deletes selected entity entries from the list (does not delete the entity itself).
        void DeleteEntitiesFromList();

        //! Remove coponent from entity and refresh property browser.
        void DeleteComponent(const QString &componentType, const QString &name);

        /// Open a dialog window that will get information from the user what type of compoent he/she wants to create.
        void CreateComponent();

        /// Deletes entity.
        void DeleteEntity();

        /// Copy serializable component values to clipboard.
        void CopyEntity();

        /// Paste create a new entity and add serializable components.
        void PasteEntity();

        /// Highlights all entities from the list that owns the component.
        void HighlightEntities(IComponent *component);

        /// If entity selection different from previous update change browser to fit those changes.
        void RefreshPropertyBrowser();

        /// Shows context menu for entities.
        /// @param pos Mouse position of right-click event.
        void ShowEntityContextMenu(const QPoint &pos);

        /// Shows EC XML editor.for entity's all components.
        void ShowXmlEditorForEntity();

        /// Shows EC XML editor.for each components.
        void ShowXmlEditorForComponent(std::vector<ComponentPtr> components);

        /// Shows EC XML editor.for a single component.
        //void ShowXmlEditorForComponent();
        void ShowXmlEditorForComponent(const std::string &componentType);

        /// Show/Hide entity list.
        void ToggleEntityList();

        /// Checks if deleted entity is located in editor's list and if so remove it from the editor.
        void EntityRemoved(Scene::Entity* entity);

    signals:
        /// Emitted user wants to edit entity's EC attributes in XML editor.
        void EditEntityXml(Scene::EntityPtr entity);

        /// Emitted user wants to edit EC attributes in XML editor.
        void EditComponentXml(ComponentPtr component);

        /// Emitted user wants to edit entity's EC attributes in XML editor.
        /// @param entities list of entities
        void EditEntityXml(const QList<Scene::EntityPtr> &entities);

        /// Emitted user wants to edit EC attributes in XML editor.
        /// @param list of components
        void EditComponentXml(const QList<ComponentPtr> & components);

    protected:
        /// QWidget override.
        void hideEvent(QHideEvent *hide_event);

        /// QWidget override.
        void changeEvent(QEvent *change_event);

    private slots:
        void SceneAdded(const QString &name);
        
        //When user have pressed ok or cancel button in component dialog this mehtod is called.
        void ComponentDialogFinnished(int result);

    private:
        void BoldEntityListItem(entity_id_t, bool bold = true);
        /// Initializes the widget.
        void Initialize();

        /// Returns string lists of EC's the ComponentManager can create.
        QStringList GetAvailableComponents() const;

        /// Returns list of selected entities.
        std::vector<Scene::EntityPtr> GetSelectedEntities() const;

        /// Framework pointer.
        Foundation::Framework *framework_;

        QPushButton* toggle_entities_button_;
        QListWidget* entity_list_;
        ECBrowser *browser_;
        typedef QMap<QString, QString> EntityIdToNameMap;
        EntityIdToNameMap entity_id_to_name_;
        typedef QSet<entity_id_t> EntityIdSet;
        EntityIdSet selectedEntities_;
        AddComponentDialog *component_dialog_;
    };
}

#endif
