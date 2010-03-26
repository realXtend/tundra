// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarEditor.h"
#include "Avatar.h"
#include "AvatarAppearance.h"

#include "SceneManager.h"
#include "EntityComponent/EC_AvatarAppearance.h"
#include "RexLogicModule.h"
#include "QtUtils.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include <QTabWidget>

#include "Inworld/InworldSceneController.h"
#include "UiModule.h"
#include "Inworld/View/UiProxyWidget.h"
#include "Inworld/View/UiWidgetProperties.h"

#include "ConfigurationManager.h"
#include "ModuleManager.h"

namespace RexLogic
{

    AvatarEditor::AvatarEditor(RexLogicModule* rexlogicmodule) :
        rexlogicmodule_(rexlogicmodule),
        avatar_widget_(0)
    {
        InitEditorWindow();

        last_directory_ = rexlogicmodule_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "last_avatar_editor_dir", std::string());
        if (last_directory_.empty())
            last_directory_ = Foundation::QtUtils::GetCurrentPath();
    }

    AvatarEditor::~AvatarEditor()
    {
        SAFE_DELETE(avatar_widget_);
        avatar_editor_proxy_widget_ = 0;
        rexlogicmodule_->GetFramework()->GetDefaultConfig().SetSetting("RexAvatar", "last_avatar_editor_dir", last_directory_);
    }

    void AvatarEditor::ExportAvatar()
    {
        rexlogicmodule_->GetAvatarHandler()->ExportUserAvatar();
    }

    void AvatarEditor::InitEditorWindow()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = rexlogicmodule_->GetFramework()->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();

        // If this occurs, we're most probably operating in headless mode.
        if (ui_module.get() == 0)
            return;

        QUiLoader loader;
        QFile file("./data/ui/avatareditor.ui");

        if (!file.exists())
        {
            RexLogicModule::LogError("Cannot find avatar editor .ui file.");
            return;
        }

        avatar_widget_ = loader.load(&file); 
        if (!avatar_widget_)
            return;

        avatar_editor_proxy_widget_ = ui_module->GetInworldSceneController()->AddWidgetToScene(avatar_widget_, UiServices::UiWidgetProperties("Avatar Editor", UiServices::ModuleWidget));

        // Connect signals.
        QPushButton *button = avatar_widget_->findChild<QPushButton *>("but_export");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(ExportAvatar()));

        button = avatar_widget_->findChild<QPushButton *>("but_load");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(LoadAvatar()));

        button = avatar_widget_->findChild<QPushButton *>("but_revert");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(RevertAvatar()));

        button = avatar_widget_->findChild<QPushButton *>("but_attachment");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddAttachment()));
    }

    void AvatarEditor::RebuildEditView()
    {
        if (!avatar_widget_)
            return;

        // Activate/deactivate export button based on whether export currently supported
        QPushButton *button = avatar_widget_->findChild<QPushButton *>("but_export");
        if (button)
            button->setEnabled(rexlogicmodule_->GetAvatarHandler()->AvatarExportSupported());

        QWidget* mat_panel = avatar_widget_->findChild<QWidget *>("panel_materials");
        QWidget* attachment_panel = avatar_widget_->findChild<QWidget *>("panel_attachments");
        if (!mat_panel || !attachment_panel)    
            return;
                
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        int width = 308-10;
        int tab_width = 302-10;
        int itemheight = 20;

        // Materials
        ClearPanel(mat_panel); 
        const AvatarMaterialVector& materials = appearance->GetMaterials();
        mat_panel->resize(width, itemheight * (materials.size() + 1));

        for (uint y = 0; y < materials.size(); ++y)
        {
            QPushButton* button = new QPushButton("Change", mat_panel);
            button->setObjectName(QString::fromStdString(ToString<int>(y))); // Material index
            button->resize(50, 20);
            button->move(width - 50, y*itemheight);
            button->show();

            QObject::connect(button, SIGNAL(clicked()), this, SLOT(ChangeTexture()));
            // If there's a texture name, use it, because it is probably more understandable than material name
            std::string texname = materials[y].asset_.name_;
            if (materials[y].textures_.size())
                texname = materials[y].textures_[0].name_;

            QLabel* label = new QLabel(QString::fromStdString(texname), mat_panel);
            label->resize(200,20);
            label->move(0, y*itemheight);
            label->show();
        }

        // Attachments
        ClearPanel(attachment_panel);
        const AvatarAttachmentVector& attachments = appearance->GetAttachments();
        attachment_panel->resize(width, itemheight * (attachments.size() + 1));

        for (uint y = 0; y < attachments.size(); ++y)
        {
            QPushButton* button = new QPushButton("Remove", attachment_panel);
            button->setObjectName(QString::fromStdString(ToString<int>(y))); // Attachment index
            button->resize(50, 20);
            button->move(width - 50, y*itemheight);  
            button->show();

            QObject::connect(button, SIGNAL(clicked()), this, SLOT(RemoveAttachment()));

            std::string attachment_name = attachments[y].name_;
            // Strip away .xml from the attachment name for slightly nicer display
            std::size_t pos = attachment_name.find(".xml");
            if (pos != std::string::npos)
                attachment_name = attachment_name.substr(0, pos);

            QLabel* label = new QLabel(QString::fromStdString(attachment_name), attachment_panel);
            label->resize(200,20);
            label->move(0, y*itemheight);
            label->show();
        }

        // Modifiers
        QTabWidget* tabs = avatar_widget_->findChild<QTabWidget *>("tab_appearance");
        if (!tabs)
            return;
        for (;;)
        {
            QWidget* tab = tabs->widget(0);
            if (!tab)
                break;
            tabs->removeTab(0);
            delete tab;
        }

        const MasterModifierVector& master_modifiers = appearance->GetMasterModifiers();
        // If no master modifiers, show the individual morph/bone controls
        if (!master_modifiers.size())
        {
            QWidget* morph_panel = GetOrCreateTabScrollArea(tabs, "Morphs");
            QWidget* bone_panel = GetOrCreateTabScrollArea(tabs, "Bones");
            if (!morph_panel || !bone_panel)
                return;

            const BoneModifierSetVector& bone_modifiers = appearance->GetBoneModifiers();
            const MorphModifierVector& morph_modifiers = appearance->GetMorphModifiers();  
            morph_panel->resize(tab_width, itemheight * (morph_modifiers.size() + 1));
            bone_panel->resize(tab_width, itemheight * (bone_modifiers.size() + 1));

            for (uint i = 0; i < bone_modifiers.size(); ++i)
            {
                QScrollBar* slider = new QScrollBar(Qt::Horizontal, bone_panel);
                slider->setObjectName(QString::fromStdString(bone_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(bone_modifiers[i].value_ * 100.0f);
                slider->resize(150, 20);
                slider->move(tab_width - 150, i * itemheight);  
                slider->show();

                QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(BoneModifierValueChanged(int)));

                QLabel* label = new QLabel(QString::fromStdString(bone_modifiers[i].name_), bone_panel);
                label->resize(100,20);
                label->move(0, i * itemheight);
                label->show();
            }

            for (uint i = 0; i < morph_modifiers.size(); ++i)
            {
                QScrollBar* slider = new QScrollBar(Qt::Horizontal, morph_panel);
                slider->setObjectName(QString::fromStdString(morph_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(morph_modifiers[i].value_ * 100.0f);
                slider->resize(150, 20);
                slider->move(tab_width - 150, i * itemheight);  
                slider->show();

                QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MorphModifierValueChanged(int)));

                QLabel* label = new QLabel(QString::fromStdString(morph_modifiers[i].name_), morph_panel);
                label->resize(100,20);
                label->move(0, i * itemheight);
                label->show();
            }
        }
        // Otherwise show only the master modifier controls
        else
        {
            std::map<std::string, uint> item_count;

            for (uint i = 0; i < master_modifiers.size(); ++i)
            {
                std::string category_name = master_modifiers[i].category_;

                if (item_count.find(category_name) == item_count.end())
                    item_count[category_name] = 0;

                QWidget* panel = GetOrCreateTabScrollArea(tabs, category_name);
                if (!panel)
                    continue;

                QScrollBar* slider = new QScrollBar(Qt::Horizontal, panel);
                slider->setObjectName(QString::fromStdString(master_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(master_modifiers[i].value_ * 100.0f);
                slider->resize(150, 20);
                slider->move(tab_width - 150, item_count[category_name] * itemheight);  
                slider->show();

                QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MasterModifierValueChanged(int)));

                QLabel* label = new QLabel(QString::fromStdString(master_modifiers[i].name_), panel);
                label->resize(100,20);
                label->move(0, item_count[category_name] * itemheight);
                label->show();

                item_count[category_name]++;
                panel->resize(tab_width, (item_count[category_name] + 1) * itemheight);
            }
        }
    }

    void AvatarEditor::ClearPanel(QWidget* panel)
    {
        QList<QWidget*> old_children = panel->findChildren<QWidget*>();
        QList<QWidget*>::iterator i = old_children.begin();
        while (i != old_children.end())
        {
            (*i)->deleteLater();
            ++i;
        }
    }

    void AvatarEditor::MorphModifierValueChanged(int value)
    {
        QScrollBar* slider = qobject_cast<QScrollBar*>(sender());
        if (!slider)
            return;
        std::string control_name = slider->objectName().toStdString();
        if (value < 0) value = 0;
        if (value > 100) value = 100;

        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        appearance->SetModifierValue(control_name, AppearanceModifier::Morph, value / 100.0f);
        rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);
    }

    void AvatarEditor::BoneModifierValueChanged(int value)
    {
        QScrollBar* slider = qobject_cast<QScrollBar*>(sender());
        if (!slider)
            return;
        std::string control_name = slider->objectName().toStdString();
        if (value < 0) value = 0;
        if (value > 100) value = 100;

        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        appearance->SetModifierValue(control_name, AppearanceModifier::Bone, value / 100.0f);
        rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);
    }

    void AvatarEditor::MasterModifierValueChanged(int value)
    {
        QScrollBar* slider = qobject_cast<QScrollBar*>(sender());
        if (!slider)
            return;
        std::string control_name = slider->objectName().toStdString();
        if (value < 0) value = 0;
        if (value > 100) value = 100;

        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        appearance->SetMasterModifierValue(control_name, value / 100.0f);
        rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);
    }

    void AvatarEditor::LoadAvatar()
    {
        const std::string filter = "Avatar description file (*.xml);;Avatar mesh (*.mesh)";
        std::string filename = GetOpenFileName(filter, "Choose avatar file");

        if (!filename.empty())
        {
            AvatarPtr avatar_handler = rexlogicmodule_->GetAvatarHandler();
            Scene::EntityPtr entity = avatar_handler->GetUserAvatar();
            if (!entity)
            {
                RexLogicModule::LogError("User avatar not in scene, cannot load appearance");
                return;
            }
            avatar_handler->GetAppearanceHandler().LoadAppearance(entity, filename);
        }
    }

    void AvatarEditor::RevertAvatar()
    {
        // Reload avatar from storage, or reload default
        rexlogicmodule_->GetAvatarHandler()->ReloadUserAvatar();
    }

    void AvatarEditor::ChangeTexture()
    {
        QPushButton* button = qobject_cast<QPushButton*>(sender());
        if (!button)
            return;

        std::string index_str = button->objectName().toStdString();
        uint index = ParseString<uint>(index_str);

        const std::string filter = "Images (*.tga; *.bmp; *.jpg; *.jpeg; *.png);;Ogre material (*.material)";
        std::string filename = GetOpenFileName(filter, "Choose texture or material");
        if (!filename.empty())
        {
            Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
            if (!entity)
                return;
                
            rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().ChangeAvatarMaterial(entity, index, filename);
            RebuildEditView();
        }
    }

    void AvatarEditor::RemoveAttachment()
    {
        QPushButton* button = qobject_cast<QPushButton*>(sender());
        if (!button)
            return;

        std::string index_str = button->objectName().toStdString();
        uint index = ParseString<uint>(index_str);    
        
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        AvatarAttachmentVector attachments = appearance->GetAttachments();
        if (index < attachments.size())
        {
            attachments.erase(attachments.begin() + index);
            appearance->SetAttachments(attachments);
            rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().SetupAppearance(entity);
            RebuildEditView();
        }
    }
    
    void AvatarEditor::AddAttachment()
    {
        const std::string filter = "Attachment description file (*.xml)";
        std::string filename = GetOpenFileName(filter, "Choose attachment file");

        if (!filename.empty())
        {
            Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
            if (!entity)
                return;
                
            rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().AddAttachment(entity, filename);
            RebuildEditView();
        }
    }
    
    QWidget* AvatarEditor::GetOrCreateTabScrollArea(QTabWidget* tabs, const std::string& name)
    {
        // Fix small clipping issue for first tab, just put space on front
        QString name_with_space = " ";
        name_with_space.append(name.c_str());
        for (uint i = 0; i < tabs->count(); ++i)
        {
            if (tabs->tabText(i) == name_with_space)
            {
                QScrollArea* tab_scroll = qobject_cast<QScrollArea*>(tabs->widget(i));
                if (!tab_scroll)
                    return 0;
                QWidget* tab_panel = tab_scroll->widget();
                return tab_panel;
            }
        }
               
        QScrollArea* tab_scroll = new QScrollArea();
        QWidget* tab_panel = new QWidget();
        
        tab_scroll->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        tab_scroll->setWidgetResizable(false);
        tab_scroll->resize(tabs->contentsRect().size());            
        tab_scroll->setWidget(tab_panel);
        tabs->addTab(tab_scroll, name_with_space);
        return tab_panel;
    }

    std::string AvatarEditor::GetOpenFileName(const std::string& filter, const std::string& prompt)
    {
        std::string filename = Foundation::QtUtils::GetOpenFileName(filter, prompt, last_directory_);
        if (!filename.empty())
        {
            boost::filesystem::path path(filename);
            std::string dirname = path.branch_path().string();
            last_directory_ = dirname;
        }
        return filename; 
    }
}
