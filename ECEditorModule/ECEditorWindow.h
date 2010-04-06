// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorWindow_h
#define incl_ECEditorModule_ECEditorWindow_h

#include "Framework.h"

#include <QWidget>

class QHideEvent;
class QShowEvent;
class QPushButton;
class QListWidget;
class QTextEdit;
class QComboBox;

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
        void SelectEntity();
        void DeleteComponent();
        void CreateComponent();
        
    protected:
        void hideEvent(QHideEvent *hide_event);
        void showEvent(QShowEvent *show_event);
        
    private:
        void Initialize();
        void RefreshAvailableComponents();
        void RefreshEntityComponents();
        void SetEntity(entity_id_t entity_id);
        
        QWidget* contents_;
        QPushButton* save_button_;
        QPushButton* revert_button_;
        QPushButton* create_button_;
        QPushButton* delete_button_;
        QListWidget* entity_list_;
        QListWidget* component_list_;
        QTextEdit* attr_edit_;
        QComboBox* create_combo_;
        
        std::set<entity_id_t> selected_entities_;
    };
}

#endif
