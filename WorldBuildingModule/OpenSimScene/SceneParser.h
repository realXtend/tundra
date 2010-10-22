// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_SceneParser_h
#define incl_WorldBuildingModule_SceneParser_h

#include "Foundation.h"
#include "Entity.h"
#include "Vector3D.h"

#include <QDomNode>

class EC_Placeable;
class EC_OpenSimPrim;

namespace WorldBuilding
{
    struct AdjustData
    {
    public:
        void SetPosition(QDomNode node)
        {
            node_position = node;
            QStringList pos_list = node_position.nodeValue().split(",");
            if (pos_list.count() < 3)
                return;
            original_pos.x = pos_list[0].toFloat();
            original_pos.y = pos_list[1].toFloat();
            original_pos.z = pos_list[2].toFloat();
        }

        entity_id_t id;
        entity_id_t parent;

        QDomNode node_position;
        QList<AdjustData*> children;

        Vector3df original_pos;
        Vector3df offset;
        bool is_child;
    };

    class SceneParser : public QObject
    {
    Q_OBJECT

    public:
        SceneParser(QObject *parent, Foundation::Framework *framework);
        virtual ~SceneParser();

    public slots:
        void ExportToFile(const QString &filename, QList<Scene::Entity *> entities);
        QByteArray ParseAndAdjust(const QByteArray &content, const Vector3df &avatar_position);

    private slots:
        QByteArray ExportXml(const QString &filename, const QList<Scene::Entity *> entity_list);
        QList<Scene::Entity*> GetAllPlaceableChildren(EC_Placeable *parent);
        void ProcessElement(QDomElement node_element, Vector3df adjust_pos);
        void AddExportData(Scene::Entity *entity);

    private:
        Foundation::Framework *framework_;
        QMap<entity_id_t, AdjustData> affected_list_;

        QString export_component_name_;

        entity_id_t processing_ent_id_;
    };
}
#endif