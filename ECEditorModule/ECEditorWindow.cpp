// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ECEditorWindow.h"
#include "ECEditorModule.h"
#include "ModuleManager.h"
#include "UiModule.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "Inworld/InworldSceneController.h"
#include "Inworld/View/UiProxyWidget.h"

#include <QVBoxLayout>
#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
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
        name_edit_(0),
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
        name_edit_ = findChild<QLineEdit*>("line_nameedit");
        
        if (entity_list_)
            QObject::connect(entity_list_, SIGNAL(itemSelectionChanged()), this, SLOT(OnEntitySelected()));
            
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(
            Foundation::Module::MT_UiServices).lock();
        if (ui_module)
            ui_module->GetInworldSceneController()->AddWidgetToScene(this, UiServices::UiWidgetProperties(contents_->windowTitle(), UiServices::ModuleWidget));
        else
            ECEditorModule::LogError("Could not add widget to scene");
    }
    
    void ECEditorWindow::OnEntitySelected()
    {
        if (entity_list_)
        {
            QListWidgetItem* item = entity_list_->item(entity_list_->currentRow());
            if (item)
            {
                entity_id_t id = (entity_id_t)item->text().toInt();
                SetEntity(id);
            }
        }
    }
    
    void ECEditorWindow::hideEvent(QHideEvent* hide_event)
    {
        ClearEntities();
        QWidget::hideEvent(hide_event);
    }
    
    void ECEditorWindow::AddEntity(entity_id_t entity_id)
    {
        if (isVisible())
        {
            selected_entities_.insert(entity_id);
            
            QString entity_id_str;
            entity_id_str.setNum((int)entity_id);
            entity_list_->addItem(entity_id_str);
            
            SetEntity(entity_id);
        }
    }
    
    void ECEditorWindow::SetEntity(entity_id_t entity_id)
    {
        //! \todo Support multiple entities. For now shows components of a single entity
        component_list_->clear();
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();

        Scene::EntityPtr entity = scene->GetEntity(entity_id);
        if (!entity)
            return;
        component_list_->clear();
        const Scene::Entity::ComponentVector& components = entity->GetComponentVector();
        for (unsigned i = 0; i < components.size(); ++i)
        {
            if (components[i])
                component_list_->addItem(QString::fromStdString(components[i]->Name()));
        }
    }
    
    void ECEditorWindow::ClearEntities()
    {
        selected_entities_.clear();
        entity_list_->clear();
    }
    
}

 