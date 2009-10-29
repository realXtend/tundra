// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "AvatarEditor.h"
#include "Avatar.h"
#include "AvatarAppearance.h"
#include "SceneManager.h"
#include "EC_AvatarAppearance.h"
#include "EC_UICanvas.h"
#include "RexLogicModule.h"
#include "QtModule.h"
#include "QtUtils.h"

#include <QtUiTools>
#include <QPushButton>
#include <QScrollArea>
#include <QLabel>
#include <QScrollBar>
#include <QTabWidget>

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
                canvas_->Show();
            else
                canvas_->Hide();
        }
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
            
/*            Foundation::ModuleSharedPtr qt_module = rexlogicmodule_->GetFramework()->GetModuleManager()->GetModule("QtModule").lock();
            QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
            if (qt_ui)
            {
                Foundation::ComponentPtr uicanvasptr = qt_ui->CreateEC_UICanvasToEntity(entity, canvas_);
                QtUI::EC_UICanvas& uicanvas = *checked_static_cast<QtUI::EC_UICanvas*>(uicanvasptr.get());
                uicanvas.SetSubmeshes(0);
            }    */                        
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

        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::Internal).lock();

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

        button = avatar_widget_->findChild<QPushButton *>("but_attachment");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(AddAttachment()));
    }
    
    void AvatarEditor::RebuildEditView()
    {   
        if (!avatar_widget_)
            return;
                                
        QWidget* mat_panel = avatar_widget_->findChild<QWidget *>("panel_materials");
        QWidget* appearance_panel = avatar_widget_->findChild<QWidget *>("panel_appearance");        
        QWidget* attachment_panel = avatar_widget_->findChild<QWidget *>("panel_attachments");        
        if (!mat_panel || !appearance_panel || !attachment_panel)    
            return;
                
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
       
        int width = 298;
        int tab_width = 292;
        int itemheight = 20;
        
        // Materials
        ClearPanel(mat_panel); 
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
        
        // Attachments
        ClearPanel(attachment_panel);          
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
        
        // Modifiers
        ClearPanel(appearance_panel);        
        QTabWidget* tabs = new QTabWidget(appearance_panel);
        tabs->resize(appearance_panel->size());
        tabs->show();
          
        const MasterModifierVector& master_modifiers = appearance.GetMasterModifiers();      
        // If no master modifiers, show the individual morph/bone controls
        if (!master_modifiers.size())
        {
            QWidget* morph_panel = GetOrCreateTabScrollArea(tabs, "Morphs");
            QWidget* bone_panel = GetOrCreateTabScrollArea(tabs, "Bones");
            if (!morph_panel || !bone_panel)            
                return;
                
            const BoneModifierSetVector& bone_modifiers = appearance.GetBoneModifiers();
            const MorphModifierVector& morph_modifiers = appearance.GetMorphModifiers();  
            morph_panel->resize(tab_width, itemheight * morph_modifiers.size());
            bone_panel->resize(tab_width, itemheight * bone_modifiers.size());
                                                      
            for (Core::uint i = 0; i < bone_modifiers.size(); ++i)
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
          
            for (Core::uint i = 0; i < morph_modifiers.size(); ++i)
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
            std::map<std::string, Core::uint> item_count;

            for (Core::uint i = 0; i < master_modifiers.size(); ++i)
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
                panel->resize(tab_width, item_count[category_name] * itemheight);
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
    
    void AvatarEditor::AddAttachment()
    {
        const std::string filter = "Attachment description file (*.xml)";
        std::string filename = Foundation::QtUtils::GetOpenFileName(filter, "Choose attachment file", Foundation::QtUtils::GetCurrentPath());      

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
        for (Core::uint i = 0; i < tabs->count(); ++i)
        {
            if (tabs->tabText(i).toStdString() == name)
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
        tab_scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        tab_scroll->setWidgetResizable(false);
        tab_scroll->resize(tabs->contentsRect().size());            
        tab_scroll->setWidget(tab_panel);    
        tabs->addTab(tab_scroll, QString::fromStdString(name));
        tab_scroll->show();
        tab_panel->show();    
        return tab_panel;
    }   
}