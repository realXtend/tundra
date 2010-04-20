// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NoteCardModule_NoteCardManager_h
#define incl_NoteCardModule_NoteCardManager_h

#include "Framework.h"
#include "Vector3D.h"

#include <QWidget>
#include <QTreeWidgetItem>

class QTreeWidget;
class QPushButton;

namespace ProtocolUtilities
{
    class WorldStream;
    typedef boost::shared_ptr<WorldStream> WorldStreamPtr;
}

namespace UiServices
{
    class UiProxyWidget;
}

namespace NoteCard
{
    class NoteCardTreeWidgetItem : public QTreeWidgetItem
    {
    public:
        NoteCardTreeWidgetItem(entity_id_t id) :
            QTreeWidgetItem(),
            id_(id)
        {
        }
        
        entity_id_t GetId() const { return id_; }
        
    private:
        entity_id_t id_;
    };
    
    class NoteCardManager : public QWidget
    {
        Q_OBJECT
        
    public:
        NoteCardManager(Foundation::Framework* framework);
        ~NoteCardManager();

        void SetWorldStream(ProtocolUtilities::WorldStreamPtr world_stream);
        void OnEntityModified(entity_id_t id);
        void OnEntityRemoved(entity_id_t id);
        void OnEntityAdded(entity_id_t id);
        void Update(f64 frametime);
        void ClearList();
        
    public slots:
        void BringToFront();
        void SelectNoteCard();
        void CreateNoteCard();
        void DeleteNoteCard();
        
    protected:
        void changeEvent(QEvent *change_event);
        
    private:
        void Initialize();

        ProtocolUtilities::WorldStreamPtr world_stream_;
        Foundation::Framework *framework_;
        QWidget* contents_;
        QTreeWidget* tree_;
        QPushButton* delete_button_;
        QPushButton* create_button_;
        UiServices::UiProxyWidget* proxy_;
        
        Vector3df new_entity_pos_;
        bool entity_create_pending_;
        entity_id_t new_entity_id_;
        Real entity_wait_time_;
        Real entity_max_wait_time_;
    };
}

#endif
