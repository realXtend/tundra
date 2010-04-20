// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "NoteCardManager.h"
#include "NoteCardModule.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "UiDefines.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "ComponentManager.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"
#include "XMLUtilities.h"
#include "SceneEvents.h"
#include "EventManager.h"
#include "EC_NoteCard.h"
#include "EntityComponent/EC_NetworkPosition.h"
#include "WorldStream.h"
#include "RexTypes.h"

#include <QApplication>
#include <QDomDocument>
#include <QVBoxLayout>
#include <QUiLoader>
#include <QFile>
#include <QTreeWidget>
#include <QPushButton>

using namespace RexTypes;

namespace NoteCard
{
    NoteCardManager::NoteCardManager(Foundation::Framework* framework) :
        framework_(framework),
        contents_(0),
        tree_(0),
        new_entity_id_(0),
        entity_create_pending_(false),
        entity_wait_time_(0.0f),
        entity_max_wait_time_(3.0f),
        proxy_(0)
    {
        Initialize();
    }
    
    NoteCardManager::~NoteCardManager()
    {
    }
    
    void NoteCardManager::Initialize()
    {
        QUiLoader loader;
        loader.setLanguageChangeEnabled(true);
        QFile file("./data/ui/notecardmanager.ui");
        file.open(QFile::ReadOnly);
        contents_ = loader.load(&file, this);
        if (!contents_)
        {
            NoteCardModule::LogError("Could not load notecard manager layout");
            return;
        }
        file.close();

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(contents_);
        layout->setContentsMargins(0,0,0,0);
        setLayout(layout);

        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(
            Foundation::Module::MT_UiServices).lock();
        if (ui_module)
        {
            UiServices::UiWidgetProperties widget_properties(contents_->windowTitle(), UiServices::SceneWidget);
            proxy_ = ui_module->GetInworldSceneController()->AddWidgetToScene(this, widget_properties);
        }
        else
            NoteCardModule::LogError("Could not add widget to scene");

        tree_ = findChild<QTreeWidget*>("tree_cards");
        if (tree_)
        {
            QObject::connect(tree_, SIGNAL(itemActivated(QTreeWidgetItem *, int)), this, SLOT(SelectNoteCard()));
        }
        create_button_ = findChild<QPushButton*>("but_create");
        if (create_button_)
        {
            QObject::connect(create_button_, SIGNAL(pressed()), this, SLOT(CreateNoteCard()));
        }
        delete_button_ = findChild<QPushButton*>("but_delete");
        if (delete_button_)
        {
            QObject::connect(delete_button_, SIGNAL(pressed()), this, SLOT(DeleteNoteCard()));
        }
    }
    
    void NoteCardManager::SetWorldStream(ProtocolUtilities::WorldStreamPtr world_stream)
    {
        world_stream_ = world_stream;
    }
    
    void NoteCardManager::BringToFront()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(
            Foundation::Module::MT_UiServices).lock();
        if (ui_module)
        {
            ui_module->GetInworldSceneController()->BringProxyToFront(this);
            ui_module->GetInworldSceneController()->ShowProxyForWidget(this);
        }
    }
    
    void NoteCardManager::changeEvent(QEvent *e)
    {
        if (e->type() == QEvent::LanguageChange)
        {
            QString title = QApplication::translate("NoteCardManager", "Notecard Manager");
            graphicsProxyWidget()->setWindowTitle(title);
        }
        else
            QWidget::changeEvent(e);
    }
    
    void NoteCardManager::SelectNoteCard()
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;
        NoteCardTreeWidgetItem* item = dynamic_cast<NoteCardTreeWidgetItem*>(tree_->currentItem());
        if (item)
        {
            entity_id_t id = item->GetId();
            
            Scene::EntityPtr entity = scene->GetEntity(id);
            if (!entity)
                return;
            boost::shared_ptr<EC_NoteCard> card = entity->GetComponent<EC_NoteCard>();
            if (card)
            {
                // Show the notecard if it was hidden
                card->Show();
            }
        }
    }
    
    void NoteCardManager::CreateNoteCard()
    {
        // Cannot create another notecard while previous create is still pending
        if (entity_create_pending_)
            return;
            
        if (!world_stream_.get())
            return;
        
        //! \todo some check to see if an object already exists dangerously close
        Vector3df pos;
        pos.x = (rand() & 32767) / 32767.0f * 255.0f;
        pos.y = (rand() & 32767) / 32767.0f * 255.0f;
        pos.z = 100.0f;
        
        new_entity_pos_ = pos;
        new_entity_id_ = 0;
        entity_create_pending_ = true;
        entity_wait_time_ = 0.0f;
        
        world_stream_->SendObjectAddPacket(pos);
    }
    
    void NoteCardManager::DeleteNoteCard()
    {
        if ((!tree_) || (!world_stream_.get()))
            return;
            
        NoteCardTreeWidgetItem* item = dynamic_cast<NoteCardTreeWidgetItem*>(tree_->currentItem());
        if (item)
        {
            entity_id_t id = item->GetId();
            world_stream_->SendObjectDeRezPacket((unsigned long)id, QString::fromStdString(RexUUID().ToString()));
        }
    }
    
    void NoteCardManager::OnEntityAdded(entity_id_t id)
    {
        if (!entity_create_pending_)
            return;
        // Take the entityID. It will not contain the necessary components yet to determine if it's ours,
        // so we check it on the next update
        new_entity_id_ = id;
    }
    
    void NoteCardManager::Update(f64 frametime)
    {
        if (!entity_create_pending_)
            return;
        // See if we have a potential entity to check
        if (new_entity_id_)
        {
            NoteCardModule::LogInfo("Checking new entity");
            Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
            Scene::EntityPtr entity;
            if (scene)
                entity = scene->GetEntity(new_entity_id_);
            if (entity)
            {
                boost::shared_ptr<RexLogic::EC_NetworkPosition> netpos = entity->GetComponent<RexLogic::EC_NetworkPosition>();
                if (netpos)
                {
                    // See if error in position is small enough to be a match
                    // Note: for some reason we can't trust the sim to give the Z position even remotely the same
                    if ((fabs(netpos->position_.x - new_entity_pos_.x) < 0.001f) &&
                        (fabs(netpos->position_.y - new_entity_pos_.y) < 0.001f) &&
                        (fabs(netpos->position_.z - new_entity_pos_.z) < 1.0f))
                    {
                        // Yes, it has to be that entity!
                        entity_create_pending_ = false;
                        // Create a new notecard EC to the entity and set its initial title, will cause sync to network
                        entity->AddComponent(framework_->GetComponentManager()->CreateComponent(EC_NoteCard::TypeNameStatic()));
                        boost::shared_ptr<EC_NoteCard> notecard = entity->GetComponent<EC_NoteCard>();
                        if (notecard)
                            notecard->SetTitle(QApplication::translate("NoteCardManager", "New notecard").toStdString());
                        else
                            NoteCardModule::LogError("Could not create notecard component to entity");
                    }
                    else
                    {
                        NoteCardModule::LogInfo("Distance was too great");
                        std::cout << netpos->position_.x << " " << netpos->position_.y << " " << netpos->position_.z << std::endl;
                        std::cout << "VS: " << new_entity_pos_.x << " " << new_entity_pos_.y << " " << new_entity_pos_.z << std::endl;
                    }
                }
                else
                    NoteCardModule::LogInfo("No networkposition EC");
            }
            new_entity_id_ = 0;
        }
        // If didn't succeed yet, increment time and see if it's time to give up
        if (entity_create_pending_)
        {
            entity_wait_time_ += frametime;
            if (entity_wait_time_ >= entity_max_wait_time_)
            {
                entity_create_pending_ = false;
                NoteCardModule::LogWarning("Wait time expired, notecard entity was not created");
            }
        }
    }
    
    void NoteCardManager::OnEntityModified(entity_id_t id)
    {
        if (!tree_)
            return;
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;
        Scene::EntityPtr entity = scene->GetEntity(id);
        if (!entity)
            return;
        boost::shared_ptr<EC_NoteCard> card = entity->GetComponent<EC_NoteCard>();
        // If entity no longer has the notecard component, trigger remove from list instead
        //! \todo this is triggered any time any entity with any serializable EC is modified. A bit unnecessary
        if (!card)
        {
            OnEntityRemoved(id);
            return;
        }
        // See if we already have this entity on our list
        for (uint i = 0; i < tree_->topLevelItemCount(); ++i)
        {
            NoteCardTreeWidgetItem* item = dynamic_cast<NoteCardTreeWidgetItem*>(tree_->topLevelItem(i));
            if (item)
            {
                // Update title from the EC
                if (item->GetId() == id)
                {
                    item->setText(0, QString::fromStdString(card->GetTitle()));
                    return;
                }
            }
        }
        // If not, create new
        NoteCardTreeWidgetItem* item = new NoteCardTreeWidgetItem(id);
        item->setText(0, QString::fromStdString(card->GetTitle()));
        tree_->addTopLevelItem(item);
    }
    
    void NoteCardManager::OnEntityRemoved(entity_id_t id)
    {
        if (!tree_)
            return;
            
        for (uint i = 0; i < tree_->topLevelItemCount(); ++i)
        {
            NoteCardTreeWidgetItem* item = dynamic_cast<NoteCardTreeWidgetItem*>(tree_->topLevelItem(i));
            if (item)
            {
                if (item->GetId() == id)
                {
                    QTreeWidgetItem* item = tree_->takeTopLevelItem(i);
                    delete item;
                    return;
                }
            }
        }
    }
    
    void NoteCardManager::ClearList()
    {
        if (tree_)
            tree_->clear();
    }
}

