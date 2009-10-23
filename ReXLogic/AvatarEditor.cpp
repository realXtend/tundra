// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarEditor.h"
#include "Avatar.h"
#include "AvatarAppearance.h"
#include "SceneManager.h"
#include "EC_AvatarAppearance.h"
#include "RexLogicModule.h"
#include "QtModule.h"
#include "QtUtils.h"

#include <QtUiTools>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>

namespace RexLogic
{

    AvatarEditor::AvatarEditor(RexLogicModule* rexlogicmodule) :
        rexlogicmodule_(rexlogicmodule),
        avatar_widget_(0)
    {
        InitEditorWindow();
    }

    AvatarEditor::~AvatarEditor()
    {
        Foundation::ModuleSharedPtr qt_module = rexlogicmodule_->GetFramework()->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
        
        if (qt_ui)
        {
            if (canvas_)
                qt_ui->DeleteCanvas(canvas_->GetID());
        }
    }
           
    void AvatarEditor::Toggle()
    {
        if (canvas_)
        {
            if (canvas_->IsHidden())
            {
                canvas_->Show();
            }
            else
                canvas_->Hide();
        }
    }
    
    void AvatarEditor::Close()
    {
        // Actually just hide the canvas
        if (canvas_)
            canvas_->Hide();
    }
    
    void AvatarEditor::ExportAvatar()
    {
        rexlogicmodule_->GetAvatarHandler()->ExportUserAvatar();
    }
    
    void AvatarEditor::LoadAvatar()
    {
        const std::string filter = "Avatar description file (*.xml);;Avatar mesh (*.mesh)";
        std::string filename = Foundation::QtUtils::GetOpenFileName(filter, "Choose avatar file", Foundation::QtUtils::GetCurrentPath());        

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
    
    void AvatarEditor::InitEditorWindow()
    {
        boost::shared_ptr<QtUI::QtModule> qt_module = rexlogicmodule_->GetFramework()->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

        // If this occurs, we're most probably operating in headless mode.
        if (qt_module.get() == 0)
            return;

        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::External).lock();

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
            
        // Set canvas size. 
        QSize size = avatar_widget_->size();
        canvas_->SetCanvasSize(size.width() + 1, size.height() + 1);
        canvas_->SetCanvasWindowTitle(QString("Avatar Editor"));
                
        canvas_->AddWidget(avatar_widget_);
        //canvas_->Show();
   
        // Set canvas scrollbar policy
        canvas_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        canvas_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);   
   
	    // Add to control bar
		qt_module->AddCanvasToControlBar(canvas_, QString("Avatar Editor"));
			           
        // Connect signals            
        QPushButton *button = avatar_widget_->findChild<QPushButton *>("but_export");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(ExportAvatar()));

        button = avatar_widget_->findChild<QPushButton *>("but_load");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(LoadAvatar()));

        button = avatar_widget_->findChild<QPushButton *>("but_revert");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(RevertAvatar()));
    }
    
    void AvatarEditor::RebuildEditView()
    {   
        if (!avatar_widget_)
            return;
                                
        QWidget* mat_panel = avatar_widget_->findChild<QWidget *>("panel_materials");
        QWidget* morph_panel = avatar_widget_->findChild<QWidget *>("panel_morphs");        
        QWidget* attachment_panel = avatar_widget_->findChild<QWidget *>("panel_attachments");        
        if (!mat_panel || !morph_panel || !attachment_panel)    
            return;
        
        ClearPanel(mat_panel); 
        ClearPanel(morph_panel);
        ClearPanel(attachment_panel);      
        
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
       
        int width = 300;
        int itemheight = 20;
        
        // Materials
        const AvatarMaterialVector& materials = appearance.GetMaterials();                
        mat_panel->resize(width, itemheight * materials.size());
        
        for (Core::uint y = 0; y < materials.size(); ++y)
        {            
            QPushButton* button = new QPushButton("Change", mat_panel);
            button->setObjectName(QString::fromStdString(Core::ToString<int>(y))); // Material index
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
        
        // Modifiers
        Core::uint y = 0;
        
        const MasterModifierVector& master_modifiers = appearance.GetMasterModifiers();      
        // If no master modifiers, show the individual morph/bone controls
        if (!master_modifiers.size())
        {       
            const BoneModifierSetVector& bone_modifiers = appearance.GetBoneModifiers();
            const MorphModifierVector& modifiers = appearance.GetMorphModifiers();  
            morph_panel->resize(width, itemheight * (modifiers.size() + bone_modifiers.size()));

            for (Core::uint i = 0; i < bone_modifiers.size(); ++i)
            {
                QScrollBar* slider = new QScrollBar(Qt::Horizontal, morph_panel);
                slider->setObjectName(QString::fromStdString(bone_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(bone_modifiers[i].value_ * 100.0f);
                slider->resize(150, 20);
                slider->move(width - 150, y*itemheight);  
                slider->show();          
                
                QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(BoneModifierValueChanged(int)));
                            
                QLabel* label = new QLabel(QString::fromStdString(bone_modifiers[i].name_), morph_panel);
                label->resize(100,20);
                label->move(0, y*itemheight);
                label->show();
                ++y;
            }            
          
            for (Core::uint i = 0; i < modifiers.size(); ++i)
            {
                QScrollBar* slider = new QScrollBar(Qt::Horizontal, morph_panel);
                slider->setObjectName(QString::fromStdString(modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(modifiers[i].value_ * 100.0f);
                slider->resize(150, 20);
                slider->move(width - 150, y*itemheight);  
                slider->show();          
                
                QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MorphModifierValueChanged(int)));
                            
                QLabel* label = new QLabel(QString::fromStdString(modifiers[i].name_), morph_panel);
                label->resize(100,20);
                label->move(0, y*itemheight);
                label->show();
                ++y;
            } 
        }
        // Otherwise show only the master modifier controls
        else
        {
            morph_panel->resize(width, itemheight * master_modifiers.size());

            for (Core::uint i = 0; i < master_modifiers.size(); ++i)
            {
                QScrollBar* slider = new QScrollBar(Qt::Horizontal, morph_panel);
                slider->setObjectName(QString::fromStdString(master_modifiers[i].name_));
                slider->setMinimum(0);
                slider->setMaximum(100);
                slider->setPageStep(10);
                slider->setValue(master_modifiers[i].value_ * 100.0f);
                slider->resize(150, 20);
                slider->move(width - 150, y*itemheight);  
                slider->show();          
                
                QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(MasterModifierValueChanged(int)));
                            
                QLabel* label = new QLabel(QString::fromStdString(master_modifiers[i].name_), morph_panel);
                label->resize(100,20);
                label->move(0, y*itemheight);
                label->show();
                ++y;
            }          
        }   
        
        // Attachments
        const AvatarAttachmentVector& attachments = appearance.GetAttachments();                
        attachment_panel->resize(width, itemheight * attachments.size());
        
        for (Core::uint y = 0; y < attachments.size(); ++y)
        {            
            QPushButton* button = new QPushButton("Remove", attachment_panel);
            button->setObjectName(QString::fromStdString(Core::ToString<int>(y))); // Attachment index
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
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
        
        appearance.SetModifierValue(control_name, AppearanceModifier::Morph, value / 100.0f);
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
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
        
        appearance.SetModifierValue(control_name, AppearanceModifier::Bone, value / 100.0f);
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
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
        
        appearance.SetMasterModifierValue(control_name, value / 100.0f);
        rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);                       
    }
        
    void AvatarEditor::ChangeTexture()
    {
        QPushButton* button = qobject_cast<QPushButton*>(sender());
        if (!button)
            return;
        
        std::string index_str = button->objectName().toStdString();
        Core::uint index = Core::ParseString<Core::uint>(index_str);
        
        const std::string filter = "Images (*.tga; *.bmp; *.jpg; *.jpeg; *.png);;Ogre material (*.material)";
        std::string filename = Foundation::QtUtils::GetOpenFileName(filter, "Choose texture or material", Foundation::QtUtils::GetCurrentPath());          
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
        Core::uint index = Core::ParseString<Core::uint>(index_str);    
        
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
        
        AvatarAttachmentVector attachments = appearance.GetAttachments();
        if (index < attachments.size())
        {
            attachments.erase(attachments.begin() + index);
            appearance.SetAttachments(attachments);
            rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().SetupAppearance(entity);
            RebuildEditView();              
        }
    }
}