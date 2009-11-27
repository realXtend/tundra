// For conditions of distribution and use, see copyright notice in license.txt

#include "UICanvasTestEdit.h"
#include "QtModule.h"
#include "SceneManager.h"
#include "EC_UICanvas.h"
#include "QtUtils.h"

#include <QtUiTools>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"

namespace QtUI
{
    UICanvasTestEdit::UICanvasTestEdit(Foundation::Framework* framework) : framework_(framework), editor_widget_(0), last_entity_id_(0)
    {   
        InitEditorWindow();    
    }

    UICanvasTestEdit::~UICanvasTestEdit()
    {
        Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
        
        if (qt_ui)
        {
            if (canvas_)
                qt_ui->DeleteCanvas(canvas_->GetID());
        }    
    }
    
    void UICanvasTestEdit::InitEditorWindow()
    {
        boost::shared_ptr<QtUI::QtModule> qt_module = framework_->GetModuleManager()->GetModule<QtUI::QtModule>(Foundation::Module::MT_Gui).lock();

        // If this occurs, we're most probably operating in headless mode.
        if (qt_module.get() == 0)
            return;

        canvas_ = qt_module->CreateCanvas(QtUI::UICanvas::Internal).lock();

        QUiLoader loader;
        QFile file("./data/ui/uicanvastestedit.ui");

        if (!file.exists())
        {
            QtModule::LogError("Cannot find UI canvas test editor .ui file.");
            return;
        }

        editor_widget_ = loader.load(&file); 
        if (!editor_widget_)
            return;
            
        // Set canvas size. 
        QSize size = editor_widget_->size();
        canvas_->SetSize(size.width() + 1, size.height() + 1);
        canvas_->SetWindowTitle(QString("3D UICanvas Test Edit"));
        canvas_->SetPosition(60,60);
                        
        canvas_->AddWidget(editor_widget_);
   
        // Set canvas scrollbar policy
        QGraphicsView* view = canvas_->GetView();
        view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff); 

	    // Add to control bar
		qt_module->AddCanvasToControlBar(canvas_, QString("3D UICanvas Test Edit"));
		
		// Connect signals   
        QPushButton *button = editor_widget_->findChild<QPushButton *>("but_bind");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(BindCanvas()));
        button = editor_widget_->findChild<QPushButton *>("but_unbind");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(UnbindCanvas()));
        QObject::connect(canvas_.get(), SIGNAL(Shown()), this, SLOT(Shown()));
        QObject::connect(canvas_.get(), SIGNAL(Hidden()), this, SLOT(Hidden()));
    }

    void UICanvasTestEdit::BindCanvas()
    {
        QComboBox* combo_s = editor_widget_->findChild<QComboBox*>("combo_subobject");    
        if (!combo_s)
            return;    
        QComboBox* combo_c = editor_widget_->findChild<QComboBox*>("combo_canvas");    
        if (!combo_c)
            return;
        Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());  
                
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (scene.get())
        {
            Scene::EntityPtr entity = scene->GetEntity(last_entity_id_);
            if (entity.get())
            {
                Core::uint submesh = combo_s->currentIndex();
                QString canvasid = combo_c->currentText();
                boost::shared_ptr<UICanvas> canvas;
                
                const QList<boost::shared_ptr<UICanvas> >& canvases = qt_ui->GetCanvases();
                for (Core::uint i = 0; i < canvases.size(); ++i)
                {
                    if (canvases[i]->GetID() == canvasid)
                        canvas = canvases[i];
                }
                
                if (canvas)
                {
                    EC_UICanvas* ec = dynamic_cast<EC_UICanvas*>(qt_ui->CreateEC_UICanvasToEntity(entity.get(), canvas).get());
                    if (ec)
                        ec->SetSubmeshes(submesh);
                }
            }
        }
                
    }
    
    void UICanvasTestEdit::UnbindCanvas()
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (scene.get())
        {
            Scene::EntityPtr entity = scene->GetEntity(last_entity_id_);
            if (entity.get())
            {
                EC_UICanvas* ec = dynamic_cast<EC_UICanvas*>(entity->GetComponent(EC_UICanvas::NameStatic()).get());
                if (ec)
                    ec->ClearSubmeshes();
            }
        }
    }
    
    void UICanvasTestEdit::Shown()
    {
        RefreshCanvases();
    }

    void UICanvasTestEdit::Hidden()
    {
        SetEntityId(0);
    }
    
    void UICanvasTestEdit::SetEntityId(Core::entity_id_t entity_id)
    {
        last_entity_id_ = entity_id;
        RefreshSubmeshes();
        
        if (!editor_widget_)
            return;
        QLabel* label = editor_widget_->findChild<QLabel*>("label_entity");       
        if (!label)
            return;
        
        if (!last_entity_id_)
            label->setText("");
        else
            label->setText(QString::fromStdString(Core::ToString<Core::uint>(last_entity_id_)));
    }
    
    void UICanvasTestEdit::RefreshSubmeshes()
    {
        if (!editor_widget_)
            return;
            
        Core::uint numsubmeshes = 0;
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (scene.get())
        {
            Scene::EntityPtr entity = scene->GetEntity(last_entity_id_);
            if (entity.get())
            {
                OgreRenderer::EC_OgreMesh* mesh = dynamic_cast<OgreRenderer::EC_OgreMesh*>(entity->GetComponent(OgreRenderer::EC_OgreMesh::NameStatic()).get());
                OgreRenderer::EC_OgreCustomObject* custom = dynamic_cast<OgreRenderer::EC_OgreCustomObject*>(entity->GetComponent(OgreRenderer::EC_OgreCustomObject::NameStatic()).get());
                if (mesh)
                    numsubmeshes = mesh->GetNumMaterials();
                if (custom)
                    numsubmeshes = custom->GetNumMaterials();
            }
        }
        
        QComboBox* combo = editor_widget_->findChild<QComboBox*>("combo_subobject");    
        if (!combo)
            return;        
        combo->clear();
        for (Core::uint i = 0; i < numsubmeshes; ++i)
            combo->addItem(QString::fromStdString(Core::ToString<Core::uint>(i)));
    }
    
    void UICanvasTestEdit::RefreshCanvases()
    {
        if (!editor_widget_)
            return;
                
        Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
        const QList<boost::shared_ptr<UICanvas> >& canvases = qt_ui->GetCanvases();
        
        QComboBox* combo = editor_widget_->findChild<QComboBox*>("combo_canvas");    
        if (!combo)
            return;
                
        combo->clear();
        for (Core::uint i = 0; i < canvases.size(); ++i)
            combo->addItem(canvases[i]->GetID());
    }
    
}