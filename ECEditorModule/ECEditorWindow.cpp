// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ECEditorWindow.h"
#include "ECEditorModule.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "ComponentManager.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/View/UiProxyWidget.h"

#include <QVBoxLayout>
#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>

namespace ECEditor
{
    ECEditorWindow::ECEditorWindow(Foundation::Framework* framework) :
        framework_(framework),
        contents_(0),
        save_button_(0),
        revert_button_(0),
        create_button_(0),
        delete_button_(0),
        entity_list_(0),
        component_list_(0),
        create_combo_(0),
        attr_edit_(0)
    {
        Initialize();
    }
    
    ECEditorWindow::~ECEditorWindow()
    {
    }
    
    void ECEditorWindow::Initialize()
    {
        QUiLoader loader;
        QFile file("./data/ui/eceditor.ui");
        file.open(QFile::ReadOnly);
        contents_ = loader.load(&file, this);
        if (!contents_)
        {
            ECEditorModule::LogError("Could not load editor layout");
            return;
        }
        file.close();

        QVBoxLayout *layout = new QVBoxLayout;
        layout->addWidget(contents_);
        layout->setContentsMargins(0,0,0,0);
        setLayout(layout);
        
        save_button_ = findChild<QPushButton*>("but_save");
        revert_button_ = findChild<QPushButton*>("but_revert");
        create_button_ = findChild<QPushButton*>("but_create");
        delete_button_ = findChild<QPushButton*>("but_delete");
        entity_list_ = findChild<QListWidget*>("list_entities");
        component_list_ = findChild<QListWidget*>("list_components");
        attr_edit_ = findChild<QTextEdit*>("text_attredit");
        create_combo_ = findChild<QComboBox*>("combo_create");
        
        if (entity_list_)
            QObject::connect(entity_list_, SIGNAL(itemSelectionChanged()), this, SLOT(SelectEntity()));
        if (delete_button_)
            QObject::connect(delete_button_, SIGNAL(pressed()), this, SLOT(DeleteComponent()));
        if (create_button_)
            QObject::connect(create_button_, SIGNAL(pressed()), this, SLOT(CreateComponent()));
        
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(
            Foundation::Module::MT_UiServices).lock();
        if (ui_module)
            ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(contents_->windowTitle(), UiServices::ModuleWidget));
        else
            ECEditorModule::LogError("Could not add widget to scene");
        
        RefreshAvailableComponents();
    }
    
    void ECEditorWindow::RefreshAvailableComponents()
    {
        // Fill the create component combo box with the types of EC's the ComponentManager can create
        Foundation::ComponentManagerPtr comp_mgr = framework_->GetComponentManager();
        const Foundation::ComponentManager::ComponentFactoryMap& factories = comp_mgr->GetComponentFactoryMap();
        if (create_combo_)
        {
            create_combo_->clear();
            Foundation::ComponentManager::ComponentFactoryMap::const_iterator i = factories.begin();
            while (i != factories.end())
            {
                create_combo_->addItem(QString::fromStdString(i->first));
                ++i;
            }
        }
    }
    
    void ECEditorWindow::SelectEntity()
    {
        if (!entity_list_)
            return;
        
        QListWidgetItem* item = entity_list_->item(entity_list_->currentRow());
        if (item)
        {
            entity_id_t id = (entity_id_t)item->text().toInt();
            SetEntity(id);
        }
    }
    
    void ECEditorWindow::DeleteComponent()
    {
        bool removed = false;
        
        if (!component_list_)
            return;

        QListWidgetItem* item = component_list_->item(component_list_->currentRow());
        if (item)
        {
            std::string component_name = item->text().toStdString();
            
            Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
            
            std::set<entity_id_t>::const_iterator i = selected_entities_.begin();
            while (i != selected_entities_.end())
            {
                Scene::EntityPtr entity = scene->GetEntity(*i);
                if (entity)
                {
                    entity->RemoveComponent(entity->GetComponent(component_name));
                    removed = true;
                }
                ++i;
            }
        }
        
        if (removed)
            RefreshEntityComponents();
    }
    
    void ECEditorWindow::CreateComponent()
    {
        if (!create_combo_)
            return;
        
        std::string name = create_combo_->currentText().toStdString();
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        
        std::set<entity_id_t>::const_iterator i = selected_entities_.begin();
        while (i != selected_entities_.end())
        {
            Scene::EntityPtr entity = scene->GetEntity(*i);
            if (entity)
            {
                // We (mis)use the GetOrCreateComponent function to avoid adding the same EC multiple times, since identifying multiple EC's of similar type
                // is problematic with current API
                entity->GetOrCreateComponent(name);
            }
            ++i;
        }
        
        RefreshEntityComponents();
    }
    
    void ECEditorWindow::hideEvent(QHideEvent* hide_event)
    {
        ClearEntities();
        QWidget::hideEvent(hide_event);
    }
    
    void ECEditorWindow::showEvent(QShowEvent* show_event)
    {
        RefreshAvailableComponents();
        QWidget::showEvent(show_event);
    }
    
    void ECEditorWindow::AddEntity(entity_id_t entity_id)
    {
        if ((isVisible()) && (entity_list_))
        {
            QString entity_id_str;
            entity_id_str.setNum((int)entity_id);
            
            int count = entity_list_->count();
            bool unique = true;
            for (int i = 0; i < count; ++i)
            {
                if (entity_list_->item(i)->text() == entity_id_str)
                {
                    unique = false;
                    entity_list_->setCurrentRow(i);
                    break;
                }
            }
            
            if (unique)
            {
                entity_list_->addItem(entity_id_str);
                entity_list_->setCurrentRow(entity_list_->count() - 1);
            }
        }
    }
    
    void ECEditorWindow::SetEntity(entity_id_t entity_id)
    {
        //! \todo Support multiple entities. For now shows components of a single entity
        selected_entities_.clear();
        selected_entities_.insert(entity_id);
        
        RefreshEntityComponents();
    }
    
    void ECEditorWindow::RefreshEntityComponents()
    {
        component_list_->clear();
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        
        std::set<entity_id_t>::const_iterator i = selected_entities_.begin();
        while (i != selected_entities_.end())
        {
            Scene::EntityPtr entity = scene->GetEntity(*i);
            if (entity)
            {
                const Scene::Entity::ComponentVector& components = entity->GetComponentVector();
                for (unsigned i = 0; i < components.size(); ++i)
                {
                    if (components[i])
                        component_list_->addItem(QString::fromStdString(components[i]->Name()));
                }
            }
            ++i;
        }
    }
    
    void ECEditorWindow::ClearEntities()
    {
        selected_entities_.clear();
        if (entity_list_)
            entity_list_->clear();
    }
}

