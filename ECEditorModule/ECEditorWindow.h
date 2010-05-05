// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorWindow_h
#define incl_ECEditorModule_ECEditorWindow_h

#include "ForwardDefines.h"
#include "CoreTypes.h"

#include <QWidget>
#include <QShortcut>
#include <map>

namespace Foundation
{
    class Framework;
}

class QHideEvent;
class QShowEvent;
class QPushButton;
class QListWidget;
class QTreeWidget;
class QTextEdit;
class QComboBox;
class QKeyEvent;

class QtTreePropertyBrowser;
class QtVariantPropertyManager;
class QtProperty;

struct EntityComponentSelection
{
    Scene::EntityPtr entity_;
    std::vector<Foundation::ComponentInterfacePtr> components_;
};

namespace ECEditor
{
    class AbstractECEditor;

    class ECEditorWindow : public QWidget
    {
        Q_OBJECT

    public:
        explicit ECEditorWindow(Foundation::Framework* framework);
        ~ECEditorWindow();

        void AddEntity(entity_id_t entity_id);
        void ClearEntities();

    public slots:
        void DeleteEntitiesFromList();
        void DeleteComponent();
        void CreateComponent();
        void RevertData();
        void SaveData();
        void RefreshEntityComponents();
        void RefreshComponentData();
        void RefreshPropertyBrowser();
        void TogglePropertiesBrowser();
        //! Begin to update spesific property
        void PropertyChanged(QtProperty *property);
        
    protected:
        void hideEvent(QHideEvent *hide_event);
        void showEvent(QShowEvent *show_event);
        void changeEvent(QEvent *change_event);

    private:
        void Initialize();
        void RefreshAvailableComponents();
        std::vector<Scene::EntityPtr> GetSelectedEntities();
        std::vector<EntityComponentSelection> GetSelectedComponents();

        Foundation::Framework *framework_;
        QWidget* contents_;
        QPushButton* save_button_;
        QPushButton* revert_button_;
        QPushButton* create_button_;
        QPushButton* delete_button_;
        QPushButton* toggle_browser_button_;
        QListWidget* entity_list_;
        QTreeWidget* component_list_;
        QTextEdit* data_edit_;
        QtTreePropertyBrowser* property_browser_;
        QtVariantPropertyManager *variantManager_;
        QComboBox* create_combo_;
        QShortcut* delete_shortcut_;
        typedef std::map<std::string, AbstractECEditor *> EditorEntityList;
        EditorEntityList editorEntityList_;
    };
}

#endif
