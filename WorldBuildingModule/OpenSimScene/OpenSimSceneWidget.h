// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_OpenSimSceneWidget_h
#define incl_WorldBuildingModule_OpenSimSceneWidget_h

#include "UiProxyWidget.h"
#include "ui_OpenSimSceneWidget.h"

#include <QList>

namespace Scene
{
    class Entity;
}

namespace WorldBuilding
{
    class OpenSimSceneWidget : public UiProxyWidget, public Ui_OpenSimSceneWidget
    {
    
    Q_OBJECT

    public:
        OpenSimSceneWidget();
        virtual ~OpenSimSceneWidget();

    public slots:
        /// Add a list of entities to the list
        void AddEntityToList(QList<Scene::Entity *> entity_list);

        /// Add a entity to the list, dublicates will be ignored
        bool AddEntityToList(const QString &ent_id, Scene::Entity *entity);
        
        /// Remove a list of entities from the list
        void RemoveEntityFromList(QList<Scene::Entity *> entity_list);

        /// Remove entity from list
        void RemoveEntityFromList(const QString &ent_id, Scene::Entity *entity);
        
        /// Get selected entity ptr list
        QList<Scene::Entity*> GetSelectedEntities() { return selected_entities_; }
        
        /// Returns if this entity id is already in the ui list
        bool IsEntityListed(const QString &ent_id);

        bool Exporting() { return exporting_; }

        void ShowFunctionality(bool enabled);

    private slots:
        void ToggleExporting();
        void ResetExporting();
        void SelectScene();
        void ExportSelected();
        void PublishFile();
        void VisibilityChange(bool visible);

    signals:
        void PublishFromFile(const QString &filename, bool adjust_pos_to_avatar);
        void ExportToFile(const QString &filename, QList<Scene::Entity *> entities);

    private:
        void ShowAllHightLights(bool visible);
        void AddHighLight(Scene::Entity *entity);
        void RemoveHightLight(Scene::Entity *entity);

        QWidget *internal_widget_;
        QList<Scene::Entity*> selected_entities_;

        bool exporting_;
    };
}
#endif