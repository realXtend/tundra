// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimSceneWidget.h"
#include "Entity.h"

#include "EC_Highlight.h"

#include <QFileDialog>
#include <QMessageBox>

namespace WorldBuilding
{
    OpenSimSceneWidget::OpenSimSceneWidget() :
        UiProxyWidget(new QWidget(), Qt::Dialog),
        exporting_(false)
    {
        internal_widget_ = widget();
        setupUi(internal_widget_);
        setWindowTitle(internal_widget_->windowTitle());

        connect(internal_widget_, SIGNAL(destroyed(QObject*)), SLOT(InternalDestroyed(QObject*)));

        label_caps_fail->hide();
        label_export_help->setText("Click 'Start Exporting' and start selecting inworld object");

        connect(button_toggle_tracking, SIGNAL(clicked()), SLOT(ToggleExporting()));
        connect(button_reset_tracking, SIGNAL(clicked()), SLOT(ResetExporting()));
        connect(button_export, SIGNAL(clicked()), SLOT(PublishFile()));
        connect(button_create_scene, SIGNAL(clicked()), SLOT(ExportSelected()));
        connect(button_browse_scene, SIGNAL(clicked()), SLOT(SelectScene()));
        connect(this, SIGNAL(Visible(bool)), SLOT(VisibilityChange(bool)));

        selected_entities_.clear();
    }

    OpenSimSceneWidget::~OpenSimSceneWidget()
    {
    }

    void OpenSimSceneWidget::InternalDestroyed(QObject *obj)
    {
        internal_widget_ = 0;
    }

    void OpenSimSceneWidget::ShowFunctionality(bool enabled)
    {
        label_caps_fail->setVisible(!enabled);

        label_title->setEnabled(enabled);
        label_title_2->setEnabled(enabled);
        label_export_help->setEnabled(enabled);
        lineedit_scene->setEnabled(enabled);
        entity_list->setEnabled(enabled);
        radioButton_av_pos->setEnabled(enabled);
        radioButton_original_pos->setEnabled(enabled);
        button_browse_scene->setEnabled(enabled);
        button_export->setEnabled(enabled);
        button_toggle_tracking->setEnabled(enabled);
        button_reset_tracking->setEnabled(enabled);
        button_create_scene->setEnabled(enabled);
    }

    void OpenSimSceneWidget::VisibilityChange(bool visible)
    {
        if (!visible && exporting_)
            button_toggle_tracking->click();
        else
            ShowAllHightLights(visible);
    }

    void OpenSimSceneWidget::AddEntityToList(QList<Scene::Entity *> entity_list)
    {
        QString ent_id;
        foreach(Scene::Entity *entity, entity_list)
        {
            ent_id = QString::number(entity->GetId());
            AddEntityToList(ent_id, entity);
        }
    }

    bool OpenSimSceneWidget::AddEntityToList(const QString &ent_id, Scene::Entity *entity)
    {
        if (IsEntityListed(ent_id))
            return false;
        entity_list->addItem(ent_id);
        selected_entities_.append(entity);
        AddHighLight(entity);
        return true;
    }

    void OpenSimSceneWidget::RemoveEntityFromList(QList<Scene::Entity *> entity_list)
    {
        QString ent_id;
        foreach(Scene::Entity *entity, entity_list)
        {
            ent_id = QString::number(entity->GetId());
            RemoveEntityFromList(ent_id, entity);
        }
    }

    void OpenSimSceneWidget::RemoveEntityFromList(const QString &ent_id, Scene::Entity *entity)
    {
        QList<QListWidgetItem*> found_items = entity_list->findItems(ent_id, Qt::MatchExactly);
        foreach(QListWidgetItem *item, found_items)
        {
            QListWidgetItem *removed = entity_list->takeItem(entity_list->row(item));
            SAFE_DELETE(removed);
        }
        RemoveHightLight(entity);
        selected_entities_.removeAll(entity);
    }

    bool OpenSimSceneWidget::IsEntityListed(const QString &ent_id)
    {
        QList<QListWidgetItem*> found_items = entity_list->findItems(ent_id, Qt::MatchExactly);
        if (found_items.count() > 0)
            return true;
        else
            return false;
    }

    void OpenSimSceneWidget::ToggleExporting()
    {
        exporting_ = !exporting_;
        ShowAllHightLights(exporting_);
        if (exporting_)
        {
            label_export_help->setText("Click 'Stop Exporting' when your set is selected");
            button_toggle_tracking->setText("Stop exporting");
        }
        else
        {
            label_export_help->setText("Click 'Export to file' to store the set you have selected \nor add more objects with 'Start Exporting'");
            button_toggle_tracking->setText("Start exporting");
        }
    }

    void OpenSimSceneWidget::ResetExporting()
    {
        foreach(Scene::Entity *entity, selected_entities_)
            RemoveHightLight(entity);
        selected_entities_.clear();
        entity_list->clear();

        if (exporting_)
            button_toggle_tracking->click();
    }

    void OpenSimSceneWidget::ShowAllHightLights(bool visible)
    {
        foreach(Scene::Entity *entity, selected_entities_)
        {
            EC_Highlight *light = entity->GetComponent<EC_Highlight>().get();
            if (!light)
                continue;
            if (visible)
                light->Show();    
            else
                light->Hide();
        }
    }

    void OpenSimSceneWidget::AddHighLight(Scene::Entity *entity)
    {
        if (!entity)
            return;
        EC_Highlight *light = dynamic_cast<EC_Highlight*>(entity->GetOrCreateComponent(EC_Highlight::TypeNameStatic(), AttributeChange::LocalOnly, false).get());
        if (light)
            light->Show();
    }

    void OpenSimSceneWidget::RemoveHightLight(Scene::Entity *entity)
    {
        if (!entity)
            return;
        entity->RemoveComponent(EC_Highlight::TypeNameStatic(), AttributeChange::LocalOnly);
    }

    void OpenSimSceneWidget::SelectScene()
    {
        QString filename = QFileDialog::getOpenFileName(internal_widget_, "Select scene", "./", "*.xml");
        if (!filename.isEmpty())
            lineedit_scene->setText(filename);
    }

    void OpenSimSceneWidget::ExportSelected()
    {
        if (selected_entities_.empty())
        {
            QMessageBox::information(internal_widget_, "No Export Entities", "There are no entities to export. Press 'Start Tracking' and click on objects to select them.");
            return;
        }
        QString filename = QFileDialog::getSaveFileName(internal_widget_, "Select export file", "./", "*.xml");
        if (filename.isEmpty())
            return;
        emit ExportToFile(filename, selected_entities_);
    }

    void OpenSimSceneWidget::PublishFile()
    {
        if (lineedit_scene->text().isEmpty())
        {
            QMessageBox::information(internal_widget_, "Import File", "Import scene file is empty, please select one first.");
            return;
        }
        emit PublishFromFile(lineedit_scene->text(), radioButton_av_pos->isChecked());
    }
}