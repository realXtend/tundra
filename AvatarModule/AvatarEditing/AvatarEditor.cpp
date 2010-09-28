// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "AvatarEditing/AvatarEditor.h"

#include "Avatar/AvatarHandler.h"
#include "Avatar/AvatarAppearance.h"
#include "EntityComponent/EC_AvatarAppearance.h"

#include "SceneManager.h"
#include "QtUtils.h"
#include "ConfigurationManager.h"
#include "ModuleManager.h"

#include <QUiLoader>
#include <QFile>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include <QTabWidget>
#include <QApplication>
#include <QVBoxLayout>
#include <QGraphicsProxyWidget>
#include <QSpacerItem>
#include <QTimer>

#include "MemoryLeakCheck.h"

namespace Avatar
{
    AvatarEditor::AvatarEditor(AvatarModule *avatar_module) :
        avatar_module_(avatar_module)
    {
        InitEditorWindow();

        last_directory_ = avatar_module_->GetFramework()->GetDefaultConfig().DeclareSetting("RexAvatar", "last_avatar_editor_dir", std::string());
        if (last_directory_.empty())
            last_directory_ = Foundation::QtUtils::GetCurrentPath();
    }

    AvatarEditor::~AvatarEditor()
    {
        avatar_module_->GetFramework()->GetDefaultConfig().SetSetting("RexAvatar", "last_avatar_editor_dir", last_directory_);
    }

    void AvatarEditor::ExportAvatar()
    {
        avatar_module_->GetAvatarHandler()->ExportUserAvatar();
    }

    void AvatarEditor::ExportAvatarLocal()
    {
        const std::string filter = "Avatar description file (*.xml)";
        std::string filename = GetSaveFileName(filter, "Save avatar description and all assets");
        if (!filename.empty())
            avatar_module_->GetAvatarHandler()->ExportUserAvatarLocal(filename);
    }
    
    void AvatarEditor::InitEditorWindow()
    {
        setupUi(this);

        // Connect signals
        connect(but_export, SIGNAL(clicked()), SLOT(ExportAvatar()));
        connect(but_exportlocal, SIGNAL(clicked()), SLOT(ExportAvatarLocal()));
        connect(but_load, SIGNAL(clicked()), SLOT(LoadAvatar()));
        connect(but_revert, SIGNAL(clicked()), SLOT(RevertAvatar()));
        connect(but_attachment, SIGNAL(clicked()), this, SLOT(AddAttachment()));

        QVBoxLayout *layout = new QVBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
        panel_materials->setLayout(layout);

        layout = new QVBoxLayout();
        layout->setContentsMargins(0,0,0,0);
        layout->setSpacing(0);
        panel_attachments->setLayout(layout);

        setWindowTitle(tr("Avatar Editor"));
    }

    void AvatarEditor::RebuildEditView()
    {
        // Activate/deactivate export button based on whether export currently supported
        but_export->setEnabled(avatar_module_->GetAvatarHandler()->AvatarExportSupported());

        // Get users avatar appearance
        Scene::EntityPtr entity = avatar_module_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;
        
        QHBoxLayout *v_box = 0;
        QPushButton *button = 0;
        QLabel *label = 0;
        QScrollBar* slider = 0;
        int total_height;

        // Materials
        ClearPanel(panel_materials); 
        const AvatarMaterialVector& materials = appearance->GetMaterials();

        QVBoxLayout *materials_layout = dynamic_cast<QVBoxLayout*>(panel_materials->layout());
        if (!materials_layout)
            return;

        for (uint y = 0; y < materials.size(); ++y)
        {
            // New horizontal layout
            v_box = new QHBoxLayout();
            v_box->setContentsMargins(6,3,6,3);
            v_box->setSpacing(6);

            // If there's a texture name, use it
            std::string texname = materials[y].asset_.name_;
            if (materials[y].textures_.size())
                texname = materials[y].textures_[0].name_;

            // Create elements
            label = new QLabel(QString::fromStdString(texname));
            label->setFixedWidth(200);

            button = new QPushButton("Change");
            button->setObjectName(QString::fromStdString(ToString<int>(y))); // Material index
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(button, SIGNAL(clicked()), SLOT(ChangeTexture()));

            // Add to layouts
            v_box->addWidget(label);
            v_box->addWidget(button);
            materials_layout->addLayout(v_box);
        }
        total_height = (materials_layout->count()) * 35;
        if (total_height < 35)
            total_height = 35;
        scroll_materials->setFixedHeight(total_height);

        // Attachments
        ClearPanel(panel_attachments);
        const AvatarAttachmentVector& attachments = appearance->GetAttachments();

        QVBoxLayout *attachments_layout = dynamic_cast<QVBoxLayout*>(panel_attachments->layout());
        if (!attachments_layout)
            return;

        for (uint y = 0; y < attachments.size(); ++y)
        {
            // New horizontal layout
            v_box = new QHBoxLayout();
            v_box->setContentsMargins(6,3,6,3);
            v_box->setSpacing(6);

            // Strip away .xml from the attachment name for slightly nicer display
            std::string attachment_name = attachments[y].name_;
            std::size_t pos = attachment_name.find(".xml");
            if (pos != std::string::npos)
                attachment_name = attachment_name.substr(0, pos);

            // Create elements
            label = new QLabel(QString::fromStdString(attachment_name));
            label->setFixedWidth(200);

            button = new QPushButton("Remove");
            button->setObjectName(QString::fromStdString(ToString<int>(y))); // Attachment index
            button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            connect(button, SIGNAL(clicked()), SLOT(RemoveAttachment()));

            // Add to layouts
            v_box->addWidget(label);
            v_box->addWidget(button);
            attachments_layout->addLayout(v_box);
        }
        total_height = (attachments_layout->count()) * 35;
        if (total_height < 35)
            total_height = 35;
        scroll_attachments->setFixedHeight(total_height);

        // Clear out all tabs
        for (;;)
        {
            QWidget* tab = tab_appearance->widget(0);
            if (!tab)
                break;
            tab_appearance->removeTab(0);
            delete tab;
        }

        // Modifiers
        // If no master modifiers, show the individual morph/bone controls
        int max_items = 0;
        const MasterModifierVector& master_modifiers = appearance->GetMasterModifiers();
        if (!master_modifiers.size())
        {
            QWidget* morph_panel = GetOrCreateTabScrollArea(tab_appearance, "Morphs");
            QWidget* bone_panel = GetOrCreateTabScrollArea(tab_appearance, "Bones");
            if (!morph_panel || !bone_panel)
                return;

            QVBoxLayout *morph_layout = new QVBoxLayout();
            morph_layout->setContentsMargins(0,0,0,0);
            morph_layout->setSpacing(0);
            morph_panel->setLayout(morph_layout);

            QVBoxLayout *bone_layout = new QVBoxLayout();
            bone_layout->setContentsMargins(0,0,0,0);
            bone_layout->setSpacing(0);
            bone_panel->setLayout(bone_layout);

            const BoneModifierSetVector& bone_modifiers = appearance->GetBoneModifiers();
            const MorphModifierVector& morph_modifiers = appearance->GetMorphModifiers();  

            for (uint i = 0; i < bone_modifiers.size(); ++i)
            {
                // New horizontal layout
                v_box = new QHBoxLayout();
                v_box->setContentsMargins(6,3,6,3);
                v_box->setSpacing(6);

                // Create elements
                label = new QLabel(QString::fromStdString(bone_modifiers[i].name_));
                label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
                label->setFixedWidth(200);

                slider = new QScrollBar(Qt::Horizontal);
                slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                slider->setFixedHeight(20);
                slider->setObjectName(QString::fromStdString(bone_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(bone_modifiers[i].value_ * 100.0f);
                connect(slider, SIGNAL(valueChanged(int)), SLOT(BoneModifierValueChanged(int)));

                // Add to layouts
                v_box->addWidget(label);
                v_box->addWidget(slider);
                bone_layout->addLayout(v_box);

                if (max_items < bone_layout->count())
                    max_items = bone_layout->count();
            }

            for (uint i = 0; i < morph_modifiers.size(); ++i)
            {
                // New horizontal layout
                v_box = new QHBoxLayout();
                v_box->setContentsMargins(6,3,6,3);
                v_box->setSpacing(6);

                // Create elements
                label = new QLabel(QString::fromStdString(morph_modifiers[i].name_));
                label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
                label->setFixedWidth(200);

                slider = new QScrollBar(Qt::Horizontal);
                slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                slider->setFixedHeight(20);
                slider->setObjectName(QString::fromStdString(morph_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(morph_modifiers[i].value_ * 100.0f);
                connect(slider, SIGNAL(valueChanged(int)), SLOT(MorphModifierValueChanged(int)));
                
                // Add to layouts
                v_box->addWidget(label);
                v_box->addWidget(slider);
                morph_layout->addLayout(v_box);

                if (max_items < morph_layout->count())
                    max_items = morph_layout->count();
            }

            // Add spacer for looks to tabs that have < max_items
            if (morph_layout->count() > bone_layout->count())
                bone_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
            else
                morph_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
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

                // Create panel
                QWidget* panel = GetOrCreateTabScrollArea(tab_appearance, category_name);
                if (!panel)
                    continue;

                QVBoxLayout *panel_layout = dynamic_cast<QVBoxLayout*>(panel->layout());
                if (!panel_layout)
                {
                    panel_layout = new QVBoxLayout();
                    panel_layout->setContentsMargins(0,0,0,0);
                    panel_layout->setSpacing(0);
                    panel->setLayout(panel_layout);
                }

                // New horizontal layout
                v_box = new QHBoxLayout();
                v_box->setContentsMargins(6,3,6,3);
                v_box->setSpacing(6);

                // Create elements
                label = new QLabel(QString::fromStdString(master_modifiers[i].name_));
                label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
                label->setFixedWidth(200);

                slider = new QScrollBar(Qt::Horizontal);
                slider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
                slider->setFixedHeight(20);
                slider->setObjectName(QString::fromStdString(master_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(master_modifiers[i].value_ * 100.0f);
                connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MasterModifierValueChanged(int)));

                // Add to layouts
                v_box->addWidget(label);
                v_box->addWidget(slider);
                panel_layout->addLayout(v_box);

                item_count[category_name]++;

                if (max_items < panel_layout->count())
                    max_items = panel_layout->count();
            }

            // Add spacer for looks to tabs that have < max_items
            std::map<std::string, uint>::const_iterator iter = item_count.begin();
            while(iter != item_count.end())
            {
                QWidget* panel = GetOrCreateTabScrollArea(tab_appearance, (*iter).first);
                if (panel)
                {
                    QVBoxLayout *panel_layout = dynamic_cast<QVBoxLayout*>(panel->layout());
                    if (panel_layout)
                    {
                        if (panel_layout->count() < max_items)
                            panel_layout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));
                    }
                }
                ++iter;
            }
        }
        total_height = max_items * 26;
        if (total_height < 26)
            total_height = 26;
        else if (total_height > 250)
            total_height = 250;
        tab_appearance->setFixedHeight(total_height + 30);

        emit EditorHideMessages();
    }

    void AvatarEditor::ClearPanel(QWidget* panel)
    {
        QLayoutItem *child, *subchild;
        while ((child = panel->layout()->takeAt(0)) != 0)
        {
            QLayout *child_layout = child->layout();
            if (child_layout)
            {
                while ((subchild = child_layout->takeAt(0)) != 0)
                {
                    QWidget *widget = subchild->widget();
                    delete subchild;
                    if (widget)
                    {
                        widget->setParent(0);
                        widget->deleteLater();
                    }
                }
            }
            delete child;
        }

        //QVBoxLayout *box_layout = dynamic_cast<QVBoxLayout*>(panel->layout());
        //if (box_layout)
        //    box_layout->addSpacerItem(new QSpacerItem(1,1, QSizePolicy::Fixed, QSizePolicy::Expanding));
    }

    void AvatarEditor::MorphModifierValueChanged(int value)
    {
        QScrollBar* slider = qobject_cast<QScrollBar*>(sender());
        if (!slider)
            return;
        std::string control_name = slider->objectName().toStdString();
        if (value < 0) value = 0;
        if (value > 100) value = 100;

        Scene::EntityPtr entity = avatar_module_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        appearance->SetModifierValue(control_name, AppearanceModifier::Morph, value / 100.0f);
        avatar_module_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);
    }

    void AvatarEditor::BoneModifierValueChanged(int value)
    {
        QScrollBar* slider = qobject_cast<QScrollBar*>(sender());
        if (!slider)
            return;
        std::string control_name = slider->objectName().toStdString();
        if (value < 0) value = 0;
        if (value > 100) value = 100;

        Scene::EntityPtr entity = avatar_module_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        appearance->SetModifierValue(control_name, AppearanceModifier::Bone, value / 100.0f);
        avatar_module_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);
    }

    void AvatarEditor::MasterModifierValueChanged(int value)
    {
        QScrollBar* slider = qobject_cast<QScrollBar*>(sender());
        if (!slider)
            return;
        std::string control_name = slider->objectName().toStdString();
        if (value < 0) value = 0;
        if (value > 100) value = 100;

        Scene::EntityPtr entity = avatar_module_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        EC_AvatarAppearance* appearance = entity->GetComponent<EC_AvatarAppearance>().get();
        if (!appearance)
            return;

        appearance->SetMasterModifierValue(control_name, value / 100.0f);
        avatar_module_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);
    }

    void AvatarEditor::changeEvent(QEvent* e)
    {
        if (e->type() == QEvent::LanguageChange)
            setWindowTitle(tr("Avatar Editor"));
        else
            QWidget::changeEvent(e);
    }

    void AvatarEditor::LoadAvatar()
    {
        const std::string filter = "Avatar description file (*.xml);;Avatar mesh (*.mesh)";
        std::string filename = GetOpenFileName(filter, "Choose avatar file");

        if (!filename.empty())
        {
            AvatarHandlerPtr avatar_handler = avatar_module_->GetAvatarHandler();
            Scene::EntityPtr entity = avatar_handler->GetUserAvatar();
            if (!entity)
            {
                AvatarModule::LogError("User avatar not in scene, cannot load appearance");
                return;
            }
            avatar_handler->GetAppearanceHandler().LoadAppearance(entity, filename);
        }
    }

    void AvatarEditor::RevertAvatar()
    {
        emit EditorStatus("Reverting all local changes to avatar...");
        // Reload avatar from storage, or reload default
        avatar_module_->GetAvatarHandler()->ReloadUserAvatar();
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
            Scene::EntityPtr entity = avatar_module_->GetAvatarHandler()->GetUserAvatar();
            if (!entity)
                return;
                
            avatar_module_->GetAvatarHandler()->GetAppearanceHandler().ChangeAvatarMaterial(entity, index, filename);
            QTimer::singleShot(250, this, SLOT(RebuildEditView()));
        }
    }

    void AvatarEditor::RemoveAttachment()
    {
        QPushButton* button = qobject_cast<QPushButton*>(sender());
        if (!button)
            return;

        std::string index_str = button->objectName().toStdString();
        uint index = ParseString<uint>(index_str);    
        
        Scene::EntityPtr entity = avatar_module_->GetAvatarHandler()->GetUserAvatar();
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
            avatar_module_->GetAvatarHandler()->GetAppearanceHandler().SetupAppearance(entity);
            QTimer::singleShot(250, this, SLOT(RebuildEditView()));
        }
    }
    
    void AvatarEditor::AddAttachment()
    {
        const std::string filter = "Attachment description file (*.xml)";
        std::string filename = GetOpenFileName(filter, "Choose attachment file");

        if (!filename.empty())
        {
            Scene::EntityPtr entity = avatar_module_->GetAvatarHandler()->GetUserAvatar();
            if (!entity)
                return;
                
            avatar_module_->GetAvatarHandler()->GetAppearanceHandler().AddAttachment(entity, filename);
            QTimer::singleShot(250, this, SLOT(RebuildEditView()));
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
               
        QWidget* tab_panel = new QWidget();
        
        QScrollArea* tab_scroll = new QScrollArea();
        tab_scroll->setWidgetResizable(true);
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
    
    std::string AvatarEditor::GetSaveFileName(const std::string& filter, const std::string& prompt)
    {
        std::string filename = Foundation::QtUtils::GetSaveFileName(filter, prompt, last_directory_);
        if (!filename.empty())
        {
            boost::filesystem::path path(filename);
            std::string dirname = path.branch_path().string();
            last_directory_ = dirname;
        }
        return filename; 
    }
}
