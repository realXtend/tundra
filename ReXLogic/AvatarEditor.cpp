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
        rexlogicmodule_(rexlogicmodule)
    {
        InitEditorWindow();
    }

    AvatarEditor::~AvatarEditor()
    {
        avatar_widget_ = 0;

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
                // Sometimes the view isn't rebuilt the first time because the avatar id doesn't match connection id???
                // In any case, rebuild when making the editor visible
                RebuildEditView();
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
        canvas_->SetCanvasSize(size.width(), size.height());
        canvas_->SetCanvasWindowTitle(QString("Avatar Editor"));

        canvas_->AddWidget(avatar_widget_);
        //canvas_->Show();
           
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
        if (!mat_panel || !morph_panel)    
            return;
        
        ClearPanel(mat_panel); 
        ClearPanel(morph_panel);      
        
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
       
        int width = 270;
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
        const MorphModifierVector& modifiers = appearance.GetMorphModifiers();
        morph_panel->resize(width, itemheight * modifiers.size());

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
            
            QObject::connect(slider, SIGNAL(sliderMoved(int)), this, SLOT(MorphValueChanged(int)));
                        
            QLabel* label = new QLabel(QString::fromStdString(modifiers[i].name_), morph_panel);
            label->resize(100,20);
            label->move(0, y*itemheight);
            label->show();
            ++y;
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
    
    void AvatarEditor::MorphValueChanged(int value)
    {
        QScrollBar* slider = qobject_cast<QScrollBar*>(sender());
        if (!slider)
            return;
        std::string morph_name = slider->objectName().toStdString();
        if (value < 0) value = 0;
        if (value > 100) value = 100;
        
        Scene::EntityPtr entity = rexlogicmodule_->GetAvatarHandler()->GetUserAvatar();
        if (!entity)
            return;
        Foundation::ComponentPtr appearanceptr = entity->GetComponent(EC_AvatarAppearance::NameStatic());
        if (!appearanceptr)
            return;
        EC_AvatarAppearance& appearance = *checked_static_cast<EC_AvatarAppearance*>(appearanceptr.get());    
        
        MorphModifierVector modifiers = appearance.GetMorphModifiers();
        for (Core::uint i = 0; i < modifiers.size(); ++i)
        {
            if (modifiers[i].name_ == morph_name)
            {
                modifiers[i].value_ = value / 100.0f;
                appearance.SetMorphModifiers(modifiers);
                rexlogicmodule_->GetAvatarHandler()->GetAppearanceHandler().SetupDynamicAppearance(entity);
                break;
            }
        }                        
    }
    
    void AvatarEditor::ChangeTexture()
    {
        QPushButton* button = qobject_cast<QPushButton*>(sender());
        if (!button)
            return;
        
        std::string index_str = button->objectName().toStdString();
        Core::uint index = Core::ParseString<Core::uint>(index_str);
        std::cout << "Texture index " << index << " change" << std::endl;    
        
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
}