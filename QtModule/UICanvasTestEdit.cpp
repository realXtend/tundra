// For conditions of distribution and use, see copyright notice in license.txt
#include "StableHeaders.h"

#include "UICanvasTestEdit.h"
#include "QtModule.h"
#include "SceneManager.h"
#include "EC_UICanvas.h"

#include "QtUtils.h"

#include <QUiLoader>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>

#include "EC_OgreMesh.h"
#include "EC_OgreCustomObject.h"

#include <UiModule.h>
#include <UiSceneManager.h>
#include <UiProxyWidget.h>
#include <UiWidgetProperties.h>

namespace QtUI
{
    UICanvasTestEdit::UICanvasTestEdit(Foundation::Framework* framework) 
        : framework_(framework), 
          editor_widget_(0),
          last_entity_id_(0)
    {   
        InitEditorWindow();    
    }

    UICanvasTestEdit::~UICanvasTestEdit()
    {
        Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
        
        if (qt_ui)
        {
            foreach(boost::shared_ptr<UICanvas> proxy_canvas, proxy_widget_canvases_)
                qt_ui->DeleteCanvas(proxy_canvas->GetID());
        }

        SAFE_DELETE(editor_widget_);
        editor_widget_proxy_ = 0;
    }
    
    void UICanvasTestEdit::InitEditorWindow()
    {
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();

        // If this occurs, we're most probably operating in headless mode.
        if (ui_module.get() == 0)
            return;

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

        editor_widget_proxy_ = ui_module->GetSceneManager()->AddWidgetToScene(editor_widget_, UiServices::UiWidgetProperties(QPointF(60,60), editor_widget_->size(), Qt::Dialog, "3D GUI"));

		// Connect signals   
        QPushButton *button = editor_widget_->findChild<QPushButton *>("but_bind");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(BindCanvas()));
        button = editor_widget_->findChild<QPushButton *>("but_unbind");
        if (button)
            QObject::connect(button, SIGNAL(clicked()), this, SLOT(UnbindCanvas()));
        QObject::connect(editor_widget_proxy_, SIGNAL(Visible(bool)), this, SLOT(Shown(bool)));
    }

    void UICanvasTestEdit::BindCanvas()
    {
        // Get ui elements
        QComboBox* combo_s = editor_widget_->findChild<QComboBox*>("combo_subobject");    
        if (!combo_s)
            return;    
        QComboBox* combo_c = editor_widget_->findChild<QComboBox*>("combo_canvas");    
        if (!combo_c)
            return;

        // Get QtModule
        Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
        QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());

        // Get UiModule
        boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();

        // Get Scene
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (scene.get())
        {
            Scene::EntityPtr entity = scene->GetEntity(last_entity_id_);
            if (entity.get())
            {
                uint submesh = combo_s->currentIndex();
                QString proxy_widget_name = combo_c->currentText();
                boost::shared_ptr<QtUI::UICanvas> canvas;
                
                // Get proxy widget for selected name
                UiServices::UiProxyWidget *proxy_widget = ui_module->GetSceneManager()->GetProxyWidget(proxy_widget_name);
                if (proxy_widget)
                {
                    ui_module->GetSceneManager()->RemoveProxyWidgetFromScene(proxy_widget);
                    proxy_widget->widget()->setWindowFlags(Qt::Widget);
                    canvas = qt_ui->CreateCanvas(QtUI::UICanvas::Internal).lock();
                    canvas->SetSize(proxy_widget->size().width(), proxy_widget->size().height());
                    canvas->SetPosition(100,100);
                    canvas->AddProxyWidget(proxy_widget);
                    canvas->Show();
                    proxy_widget->show();

                    proxy_widget_canvases_.append(canvas);

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
                EC_UICanvas* ec = entity->GetComponent<EC_UICanvas>().get();
                if (ec)
                {
                    // Get proxy widget from 3D canvas
                    boost::shared_ptr<QtUI::UICanvas> canvas = ec->GetCanvas();
                    QGraphicsProxyWidget *proxy_widget = canvas->Remove3DProxyWidget();
                    proxy_widget->widget()->setWindowFlags(Qt::Dialog);

                    ec->ClearSubmeshes();
                    
                    // Remove proxy widget from local QList so it we wont try to delete in decostructor
                    proxy_widget_canvases_.removeOne(canvas);

                    // Get QtModule and delete the 3D canvas
                    Foundation::ModuleSharedPtr qt_module = framework_->GetModuleManager()->GetModule("QtModule").lock();
                    QtUI::QtModule *qt_ui = dynamic_cast<QtUI::QtModule*>(qt_module.get());
                    if (qt_ui)
                        qt_ui->DeleteCanvas(canvas->GetID());

                    // Add proxy widget back to 2D scene
                    boost::shared_ptr<UiServices::UiModule> ui_module = framework_->GetModuleManager()->GetModule<UiServices::UiModule>(Foundation::Module::MT_UiServices).lock();
                    ui_module->GetSceneManager()->AddProxyWidget((UiServices::UiProxyWidget *)proxy_widget);

                }
            }
        }
    }
    
    void UICanvasTestEdit::Shown(bool visible)
    {
        if (visible)
            RefreshCanvases();
        else
            SetEntityId(0);
    }
    
    void UICanvasTestEdit::SetEntityId(entity_id_t entity_id)
    {
        if (!editor_widget_)
            return;
    
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
            label->setText(QString::fromStdString(ToString<uint>(last_entity_id_)));
    }
    
    void UICanvasTestEdit::RefreshSubmeshes()
    {
        if (!editor_widget_)
            return;
            
        uint numsubmeshes = 0;
        
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (scene.get())
        {
            Scene::EntityPtr entity = scene->GetEntity(last_entity_id_);
            if (entity.get())
            {
                OgreRenderer::EC_OgreMesh* mesh = entity->GetComponent<OgreRenderer::EC_OgreMesh>().get();
                OgreRenderer::EC_OgreCustomObject* custom = entity->GetComponent<OgreRenderer::EC_OgreCustomObject>().get();
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
        for (uint i = 0; i < numsubmeshes; ++i)
            combo->addItem(QString::fromStdString(ToString<uint>(i)));
    }
    
    void UICanvasTestEdit::RefreshCanvases()
    {
        if (!editor_widget_)
            return;
                
        Foundation::ModuleSharedPtr ui_module = framework_->GetModuleManager()->GetModule("UiServices").lock();

        UiServices::UiModule *ui_services = dynamic_cast<UiServices::UiModule*>(ui_module.get());
        const QList<UiServices::UiProxyWidget *> proxy_widgets = ui_services->GetSceneManager()->GetAllProxyWidgets();
        
        QComboBox* combo = editor_widget_->findChild<QComboBox*>("combo_canvas");    
        if (!combo)
            return;
                
        combo->clear();
        foreach(UiServices::UiProxyWidget *proxy_widget, proxy_widgets)
            combo->addItem(proxy_widget->getWidgetProperties().getWidgetName());
    }
    
}
