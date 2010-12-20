// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_OpenSimSceneExporter_h
#define incl_WorldBuildingModule_OpenSimSceneExporter_h

#include "Foundation.h"

#include <QWidget>
#include <QObject>
#include <QString>
#include <QDir>
#include <QHash>

#include "ui_OpenSimSceneBackupWidget.h"

class UiProxyWidget;

namespace WorldBuilding
{
    class SceneParser;

    class SceneExporter : public QObject
    {
    
    Q_OBJECT

    public:
        SceneExporter(QObject *parent, Foundation::Framework *framework, SceneParser *scene_parser);
        virtual ~SceneExporter();

    public slots:
        void PostInitialize();
        void ShowBackupTool();
        void StoreSceneAndAssets(QDir store_location, const QString &asset_base_url);
        void EnableBackupButton() { backup_button_enabled_ = true; };
        void DisableBackupButton() { backup_button_enabled_ = false; };

    private slots:
        void BrowseStoreLocation();
        void StartBackup();
        void InternalDestroyed(QObject *object);
        QHash<QString, QString> ProcessAssetsRefs(QDir store_location, const QString &asset_base_url, const QString &subfolder, const QString &asset_type, QSet<QString> *ref_set);
        int ReplaceMaterialTextureRefs(const QDir &store_location_textures,  const QString &asset_base_url, const QString &material_file_path);
        int ReplaceParticleMaterialRefs(const QDir store_location_particles, const QString &asset_base_url, const QString &particle_file_path);
        QString TryStoreTexture(const QString &original_ref, const QDir &store_location, const QString &base_url);
        QString GetCacheFilename(const QString &asset_id, const QString &type);
        void CleanLocalDir(QDir dir);
        void LogHeadline(const QString &bold, const QString &msg = QString());
        void Log(const QString &message);
        void LogLineEnd();

    private:
        Foundation::Framework *framework_;
        SceneParser *scene_parser_;

        UiProxyWidget *backup_proxy_;
        QWidget *backup_widget_;
        Ui_SceneBackupWidget ui_;

        bool backup_meshes_;
        bool backup_textures_;
        bool backup_animations_;
        bool backup_particles_;
        bool backup_sounds_;
        bool backup_button_enabled_;
    };
}

#endif 