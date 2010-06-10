/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   ECEditorWindow.h
 *  @brief  Entity-component editor window.
 */

#ifndef incl_ECEditorModule_ECEditorWindow_h
#define incl_ECEditorModule_ECEditorWindow_h

#include "ForwardDefines.h"
#include "CoreTypes.h"
#include <set>

#include <QWidget>

namespace Foundation
{
    class Framework;
    class AttributeInterface;
}

QT_BEGIN_NAMESPACE;
class QPushButton;
class QListWidget;
class QTreeWidget;
class QPoint;
QT_END_NAMESPACE;

struct EntityComponentSelection
{
    Scene::EntityPtr entity;
    std::vector<Foundation::ComponentInterfacePtr> components;
};

namespace ECEditor
{
    class AttributeBrowser;

    class ECEditorWindow : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor
        /// @param framework Framework.
        explicit ECEditorWindow(Foundation::Framework* framework);

        /// Destructor.
        ~ECEditorWindow();

        /// Adds new entity to the entity list.
        void AddEntity(entity_id_t entity_id);

        /// Removes entity from the entity list.
        void RemoveEntity(entity_id_t entity_id);

        /// Clears entity list.
        void ClearEntities();

    public slots:
        /// Deletes selected entity entries from the list (does not delete the entity itself).
        void DeleteEntitiesFromList();

        //! Remove coponent from entity and refresh property browser.
        void DeleteComponent(const std::string &componentType);
        /// Deletes currently selected components.
        //void DeleteComponent();

        ///
        void CreateComponent();

        /// Deletes entity.
        void DeleteEntity();

        ///
        //void RefreshEntityComponents();

        ///
        void RefreshPropertyBrowser();

        /// Shows context menu for entities.
        /// @param pos Mouse position of right-click event.
        void ShowEntityContextMenu(const QPoint &pos);

        /// Shows context menu for components.
        /// @param pos Mouse position of right-click event.
        void ShowComponentContextMenu(const QPoint &pos);

        /// Shows EC XML editor.for entity's all components.
        void ShowXmlEditorForEntity();

        /// Shows EC XML editor.for each components.
        void ShowXmlEditorForComponent(std::vector<Foundation::ComponentInterfacePtr> components);

        /// Shows EC XML editor.for a single component.
        //void ShowXmlEditorForComponent();
        void ShowXmlEditorForComponent(const std::string &componentType);

        /// Show/Hide entity list.
        void ToggleEntityList();

    signals:
        /// Emitted user wants to edit entity's EC attributes in XML editor.
        void EditEntityXml(Scene::EntityPtr entity);

        /// Emitted user wants to edit EC attributes in XML editor.
        void EditComponentXml(Foundation::ComponentPtr component);

    protected:
        /// QWidget override.
        void hideEvent(QHideEvent *hide_event);

        /// QWidget override.
        void changeEvent(QEvent *change_event);

    private:
        /// Initializes the widget.
        void Initialize();

        /// Returns string lists of EC's the ComponentManager can create.
        QStringList GetAvailableComponents() const;

        /// Returns list of selected entities.
        std::vector<Scene::EntityPtr> GetSelectedEntities();

        /// Returns list of selected components.
        //std::vector<EntityComponentSelection> GetSelectedComponents();

        /// Framework pointer.
        Foundation::Framework *framework_;

        QPushButton* toggle_entities_button_;
        QListWidget* entity_list_;
        //QTreeWidget* component_list_;
        AttributeBrowser* attribute_browser_;
        typedef std::set<entity_id_t> EntityIdSet;
        EntityIdSet selectedEntities_;
    };
}

#endif
