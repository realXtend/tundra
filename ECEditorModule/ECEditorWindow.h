// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorWindow_h
#define incl_ECEditorModule_ECEditorWindow_h

#include "Framework.h"

#include <QWidget>
#include <QShortcut>

class QHideEvent;
class QShowEvent;
class QPushButton;
class QListWidget;
class QTextEdit;
class QComboBox;
class QKeyEvent;

namespace UiServices
{
    class UiProxyWidget;
}

struct EntityComponentSelection
{
    Scene::EntityPtr entity_;
    std::vector<Foundation::ComponentInterfacePtr> components_;
};

namespace ECEditor
{
    class ECEditorWindow : public QWidget
    {
        Q_OBJECT
        
    public:
        ECEditorWindow(Foundation::Framework* framework);
        ~ECEditorWindow();
        
        Foundation::Framework *framework_;
        
        void AddEntity(entity_id_t entity_id);
        void ClearEntities();
        
    public slots:
        void BringToFront();
        void DeleteEntitiesFromList();
        void DeleteComponent();
        void CreateComponent();
        void RevertData();
        void SaveData();
        void RefreshEntityComponents();
        void RefreshComponentData();
        
    protected:
        void hideEvent(QHideEvent *hide_event);
        void showEvent(QShowEvent *show_event);
        void changeEvent(QEvent *change_event);
        
    private:
        void Initialize();
        void RefreshAvailableComponents();
        std::vector<Scene::EntityPtr> GetSelectedEntities();
        std::vector<EntityComponentSelection> GetSelectedComponents();
        
        QWidget* contents_;
        QPushButton* save_button_;
        QPushButton* revert_button_;
        QPushButton* create_button_;
        QPushButton* delete_button_;
        QListWidget* entity_list_;
        QListWidget* component_list_;
        QTextEdit* data_edit_;
        QComboBox* create_combo_;
        QShortcut* delete_shortcut_;
        UiServices::UiProxyWidget* proxy_;
    };
}

#endif
