// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "OpenSimSceneExporter.h"
#include "OpenSimSceneService.h"
#include "WorldBuildingModule.h"
#include "SceneParser.h"

#include "UiServiceInterface.h"
#include "UiProxyWidget.h"

#include "TextureServiceInterface.h"
#include "TextureResource.h"

#include "OgreImage.h"

#include <QHash>
#include <QSet>
#include <QDateTime>
#include <QFile>
#include <QFileInfo>
#include <QImage>

#include <QFileDialog>
#include <QMessageBox>
#include <QGraphicsScene>
#include <QCryptographicHash>
#include <QUrl>
#include <QMessageBox>

namespace WorldBuilding
{

    SceneExporter::SceneExporter(QObject *parent, Foundation::Framework *framework, SceneParser *scene_parser) :
        QObject(parent),
        framework_(framework),
        scene_parser_(scene_parser),
        backup_widget_(new QWidget()),
        backup_proxy_(0),
        backup_button_enabled_(false)
    {
        ui_.setupUi(backup_widget_);

        connect(ui_.button_do_backup, SIGNAL(clicked()), SLOT(StartBackup()));
        connect(ui_.button_browse_location, SIGNAL(clicked()), SLOT(BrowseStoreLocation()));
        connect(backup_widget_, SIGNAL(destroyed(QObject*)), SLOT(InternalDestroyed(QObject*)));

        backup_meshes_ = true;
        backup_textures_ = true;
        backup_animations_ = true;
        backup_particles_ = true;
        backup_sounds_ = true;
    }

    SceneExporter::~SceneExporter()
    {
        if (backup_proxy_)
        {
            UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
            if (ui)
            {
                backup_proxy_->hide();
                ui->RemoveWidgetFromScene(backup_proxy_);
            }
        }
        if (backup_widget_)
            backup_widget_->deleteLater();
    }

    void SceneExporter::InternalDestroyed(QObject *object)
    {
        backup_widget_ = 0;
    }

    void SceneExporter::PostInitialize()
    {
        UiServiceInterface *ui = framework_->GetService<UiServiceInterface>();
        if (ui)
        {
            backup_proxy_ = ui->AddWidgetToScene(backup_widget_);
            backup_widget_->hide();
        }
        else
            WorldBuildingModule::LogWarning("OpenSimSceneService: Failed to add OpenSim Scene Tool to scene and menu");
    }

    void SceneExporter::ShowBackupTool()
    {
        OpenSimSceneService* scene_service = qobject_cast<OpenSimSceneService*>(parent());
        if (scene_service)
            scene_service->BackupToolButtonEnableRequest();

        if (!backup_proxy_)
            return;
        if (backup_proxy_->scene()->isActive() && !backup_proxy_->isVisible())
            backup_proxy_->show();
    }

    void SceneExporter::BrowseStoreLocation()
    {
        QString location = QFileDialog::getExistingDirectory(backup_widget_, "Browse Store Location");
        if (location.isEmpty())
            return;
        ui_.lineedit_store_location->setText(location);
    }

    void SceneExporter::StartBackup()
    {
        if (!backup_button_enabled_)
        {
            QMessageBox* message = new QMessageBox("Accecc denied", "You don't have permission to excecute this command.", QMessageBox::Critical, QMessageBox::Ok, 0, 0, 0);
            message->show();
            return;
        }

        backup_meshes_ = ui_.meshCheckBox->isChecked();
        backup_textures_ = ui_.texturesCheckBox->isChecked();
        backup_animations_ = ui_.animationsCheckBox->isChecked();
        backup_particles_ = ui_.particleScriptsCheckBox->isChecked();
        backup_sounds_ = ui_.soundsCheckBox->isChecked();

        QString input_location = ui_.lineedit_store_location->text();
        if (input_location.isEmpty())
        {
            QMessageBox::information(backup_widget_, "Invalid directory", "Given store directory is a empty string.");
            return;
        }
        QDir store_dir(input_location);
        if (!store_dir.exists())
        {
            QMessageBox::information(backup_widget_, "Invalid directory", "Given store directory does not exist");
            return;
        }

        QUrl store_url(ui_.lineedit_base_url->text(), QUrl::TolerantMode);
        if (!store_url.isValid())
        {
            QMessageBox::information(backup_widget_, "Invalid URL", "Given asset base URL is invalid.");
            return;
        }
        QString base_url = store_url.toString().toLower();
        if (base_url.endsWith("/"))
            base_url = base_url.left(base_url.length()-1);

        ui_.log->clear();
        StoreSceneAndAssets(store_dir, base_url);
    }

    void SceneExporter::LogHeadline(const QString &bold, const QString &msg)
    {
        ui_.log->appendHtml(QString("<b>%1</b> %2").arg(bold, msg));
    }

    void SceneExporter::Log(const QString &message)
    {
         ui_.log->appendHtml(message);
    }

    void SceneExporter::LogLineEnd()
    {
        ui_.log->appendPlainText(" ");
    }

    void SceneExporter::StoreSceneAndAssets(QDir store_location, const QString &asset_base_url)
    {
        WorldBuildingModule::LogDebug(QString("SceneExport: Storing scene with assets to %1 with asset base url %2").arg(store_location.absolutePath(),asset_base_url).toStdString().c_str());
        QString filename = store_location.absolutePath() + "/scene.xml";
        QString subfolder, asset_type;

        LogHeadline("Asset base URL  :", asset_base_url);
        LogHeadline("Store directory :", store_location.absolutePath());       

        // Cleanup target directory
        CleanLocalDir(store_location);

        // Needed data
        QSet<QString> *mesh_ref_set = new QSet<QString>();
        QSet<QString> *animation_ref_set = new QSet<QString>();
        QSet<QString> *particle_ref_set = new QSet<QString>();
        QSet<QString> *sound_ref_set = new QSet<QString>();
        QHash<QString, uint> *material_ref_set = new QHash<QString, uint>();

        QHash<QString, QString> old_to_new_references;

        // Pass storage containers for scene parser
        scene_parser_->mesh_ref_set = mesh_ref_set;
        scene_parser_->animation_ref_set = animation_ref_set;
        scene_parser_->material_ref_set = material_ref_set;
        scene_parser_->particle_ref_set = particle_ref_set;
        scene_parser_->sound_ref_set = sound_ref_set;

        // Parse scene, this will populate our sets
        Log("-- Iterating all entities for export");
        LogLineEnd();
        
        QByteArray scene_data = scene_parser_->ExportSceneXml();
        
        // Meshes
        if (backup_meshes_)
        {
            asset_type = "Mesh";
            subfolder = "meshes";
            old_to_new_references.unite(ProcessAssetsRefs(store_location, asset_base_url, subfolder, asset_type, mesh_ref_set));
        }
        else
            Log("-- Skipping mesh export");

        // Animations
        if (backup_animations_)
        {
            asset_type = "Skeleton";
            subfolder = "animations";
            old_to_new_references.unite(ProcessAssetsRefs(store_location, asset_base_url, subfolder, asset_type, animation_ref_set));
        }
        else
            Log("-- Skipping animation export");

        // Sound
        if (backup_sounds_)
        {
            asset_type = "SoundVorbis";
            subfolder = "audio";
            old_to_new_references.unite(ProcessAssetsRefs(store_location, asset_base_url, subfolder, asset_type, sound_ref_set));
        }
        else
            Log("-- Skipping sound export");

        // Material/texture processing
        if (backup_textures_)
        {
            QDir store_location_textures(store_location);
            store_location_textures.mkdir("textures");

            asset_type = "MaterialScript";
            subfolder = "scripts";
            store_location.mkdir(subfolder);
            if (store_location.cd(subfolder) && store_location_textures.cd("textures"))
            {
                WorldBuildingModule::LogDebug("SceneExport: Processing textures and materials");
                LogHeadline("Processing", "textures and materials");
                CleanLocalDir(store_location_textures);
                CleanLocalDir(store_location);
                Log("-- Parsing material scripts to resolve texture references");
                
                int found_tex = 0;
                int found_mat = 0;
                int failed_tex = 0;
                int failed_mat = 0;
                int not_found_tex = 0;
                int not_found_mat = 0;
                int replaced_material_texture_refs = 0;

                Foundation::TextureServiceInterface *texture_service = framework_->GetService<Foundation::TextureServiceInterface>();
                if (texture_service)
                {
                    foreach (QString mat_ref, material_ref_set->keys())
                    {
                        uint mat_type = material_ref_set->value(mat_ref);

                        // Texture
                        if (mat_type == 0) 
                        {
                            QString replace_url = TryStoreTexture(mat_ref, store_location_textures, asset_base_url);
                            if (!replace_url.isEmpty() && replace_url != "DYNAMIC_TEXTURE_CREATION_FAILED")
                            {
                                old_to_new_references[mat_ref] = replace_url;
                                found_tex++;
                            }
                            else if (replace_url == "DYNAMIC_TEXTURE_CREATION_FAILED")
                                failed_tex++;
                            else if (replace_url.isEmpty())
                                not_found_tex++;
                        }
                        // Material script
                        else if (mat_type == 45) 
                        {
                            QString cache_path = GetCacheFilename(mat_ref, asset_type);
                            if (QFile::exists(cache_path))
                            {      
                                QString file_only = cache_path.split("/").last().toLower();
                                if (file_only.isEmpty())
                                    continue;
                                if (file_only.endsWith(".materialscript"))
                                    file_only.replace(".materialscript", ".material");
                                store_location.remove(file_only);
                                QString new_filename = store_location.absoluteFilePath(file_only);
                                if (QFile::copy(cache_path, new_filename))
                                {
                                    old_to_new_references[mat_ref] = asset_base_url + "/scripts/" + file_only;
                                    found_mat++;
            
                                    // Replace all texture refs inside material scipts
                                    int replaced = ReplaceMaterialTextureRefs(store_location_textures, asset_base_url, new_filename);
                                    replaced_material_texture_refs += replaced;
                                }
                                else
                                {
                                    WorldBuildingModule::LogError(">> Failed to copy " + file_only.toStdString());
                                    failed_mat++;
                                }
                            }
                            else
                                not_found_mat++;
                        }
                        else
                            WorldBuildingModule::LogWarning(">> Skipping due unknown type for material/texture: " + QString::number(mat_type).toStdString());
                    }

                    // Report findings
                    WorldBuildingModule::LogDebug("SceneExport: Textures");
                    LogLineEnd();
                    LogHeadline("Textures");
                    if (not_found_tex > 0 || failed_tex > 0)
                    {
                        WorldBuildingModule::LogDebug(">> Found     : " + QString::number(found_tex).toStdString());
                        WorldBuildingModule::LogDebug(">> Not Found : " + QString::number(not_found_tex).toStdString());
                        WorldBuildingModule::LogDebug(">> Failed    : " + QString::number(failed_tex).toStdString());

                        LogHeadline(">> Found     :", QString::number(found_tex));
                        LogHeadline(">> Not Found :", QString::number(not_found_tex));
                        LogHeadline(">> Failed    :", QString::number(failed_tex));
                    }
                    else if (not_found_tex == 0 && failed_tex == 0 && found_tex == 0)
                    {
                        WorldBuildingModule::LogDebug(">> There were no textures in this scene");
                    }
                    else if (not_found_tex == 0 && failed_tex == 0 && found_tex > 0)
                    {
                        WorldBuildingModule::LogDebug(">> All Found : " + QString::number(found_tex).toStdString());
                        LogHeadline(">> All found :", QString::number(found_tex));
                    }

                    WorldBuildingModule::LogDebug("SceneExport: Materials");
                    LogLineEnd();
                    LogHeadline("Materials");
                    if (not_found_mat > 0 || failed_mat > 0)
                    {
                        WorldBuildingModule::LogDebug(">> Found     : " + QString::number(found_mat).toStdString());
                        WorldBuildingModule::LogDebug(">> Not Found : " + QString::number(not_found_mat).toStdString());
                        WorldBuildingModule::LogDebug(">> Failed    : " + QString::number(failed_mat).toStdString());

                        LogHeadline(">> Found     :", QString::number(found_mat));
                        LogHeadline(">> Not Found :", QString::number(not_found_mat));
                        LogHeadline(">> Failed    :", QString::number(failed_mat));
                    }
                    else if (not_found_mat == 0 && failed_mat == 0 && found_mat == 0)
                    {
                        WorldBuildingModule::LogDebug(">> There were no matrial scripts in this scene");
                    }
                    else if (not_found_mat == 0 && failed_mat == 0 && found_mat > 0)
                    {
                        WorldBuildingModule::LogDebug(">> All Found : " + QString::number(found_mat).toStdString());
                        LogHeadline(">> All Found :", QString::number(found_mat));
                    }
                    LogHeadline(">> Replaced texture references:" , QString::number(replaced_material_texture_refs));
                }
                else
                    WorldBuildingModule::LogError(">> Texture service not accessible, aborting processing");
                store_location.cdUp();
            }
            else
                WorldBuildingModule::LogError(">> Failed to create textures and scripts folders, aborting processing");
            LogLineEnd();
        }
        else
            Log("-- Skipping texture and material script export");

        // Particle processing
        if (backup_particles_)
        {
            asset_type = "ParticleScript";
            subfolder = "scripts";
            old_to_new_references.unite(ProcessAssetsRefs(store_location, asset_base_url, subfolder, asset_type, particle_ref_set));
        }
        else
            Log("-- Skipping particle script export");

        // Replace old references references
        QString new_ref;
        WorldBuildingModule::LogDebug("SceneExport: Replacing old asset refs with base URL " + asset_base_url.toStdString());
        LogLineEnd();
        Log("Replacing all old references using given base URL");
        foreach(QString old_ref, old_to_new_references.keys())
        {
            new_ref = old_to_new_references[old_ref];
            scene_data.replace(old_ref, new_ref.toStdString().c_str());
        }

        // Store scene data to file
        QFile scene_file(filename);
        if (scene_file.open(QIODevice::ReadWrite|QIODevice::Text))
        {
            WorldBuildingModule::LogDebug("SceneExport: Writing scene.xml file with new URL refs");
            Log("Saving scene.xml");

            scene_file.resize(0); // empty content
            scene_file.write(scene_data);
            scene_file.close();
        }
        else
        {
            WorldBuildingModule::LogError("SceneExport: Failed to open output file, coult not write XML.");
            LogHeadline("Failed to write scene.xml - I/O error!");
        }

        // Cleanup
        scene_parser_->mesh_ref_set = 0;
        scene_parser_->animation_ref_set = 0;
        scene_parser_->material_ref_set = 0;
        scene_parser_->particle_ref_set = 0;
        scene_parser_->sound_ref_set = 0;

        mesh_ref_set->clear();
        material_ref_set->clear();
        animation_ref_set->clear();
        particle_ref_set->clear();
        sound_ref_set->clear();

        SAFE_DELETE(mesh_ref_set);
        SAFE_DELETE(material_ref_set);
        SAFE_DELETE(animation_ref_set);
        SAFE_DELETE(particle_ref_set);
        SAFE_DELETE(sound_ref_set);

        WorldBuildingModule::LogDebug("SceneExport: Scene backup with assets done");
        LogHeadline("Done");
    }

    QHash<QString, QString> SceneExporter::ProcessAssetsRefs(QDir store_location, const QString &asset_base_url, const QString &subfolder, const QString &asset_type, QSet<QString> *ref_set)
    {
        QHash<QString, QString> ref_to_file;
        store_location.mkdir(subfolder);
        if (store_location.cd(subfolder))
        {
            WorldBuildingModule::LogDebug("SceneExport: Processing " + subfolder.toStdString());
            LogHeadline("Processing ", subfolder + " / " + asset_type);

            if (asset_type != "ParticleScript")
                CleanLocalDir(store_location);

            // Copy from cache to target dir
            int copied = 0;
            int not_found = 0;
            int failed = 0;
            int replaced = -1;

            foreach(QString ref, *ref_set)
            {
                QString cache_path = GetCacheFilename(ref, asset_type);
                if (QFile::exists(cache_path))
                {
                    QString file_only = cache_path.split("/").last().toLower();
                    if (file_only.isEmpty())
                    {
                        failed++;
                        continue;
                    }
                    
                    if (file_only.endsWith(".particlescript"))
                        file_only.replace(".particleScript", ".particle");
                    if (file_only.endsWith(".soundvorbis"))
                        file_only.replace(".soundvorbis", ".ogg");

                    store_location.remove(file_only);
                    QString new_filename = store_location.absoluteFilePath(file_only);
                    if (QFile::copy(cache_path, new_filename))
                    {
                        ref_to_file[ref] = asset_base_url + "/" + subfolder + "/" + file_only;
                        copied++;

                        if (asset_type == "ParticleScript")
                        {
                            if (replaced == -1)
                                replaced = 0;

                            int replaced_count = ReplaceParticleMaterialRefs(store_location, asset_base_url, new_filename);
                            replaced += replaced_count;
                        }
                    }
                    else
                    {
                        WorldBuildingModule::LogError(">> Failed to copy " + file_only.toStdString());
                        failed++;
                    }
                }
                else
                    not_found++;
            }

            // Report findings
            if (not_found > 0 || failed > 0)
            {
                WorldBuildingModule::LogDebug(">> Found     : " + QString::number(copied).toStdString());
                WorldBuildingModule::LogDebug(">> Not Found : " + QString::number(not_found).toStdString());
                WorldBuildingModule::LogDebug(">> Failed    : " + QString::number(failed).toStdString());

                LogHeadline(">> Found     :", QString::number(copied));
                LogHeadline(">> Not Found :", QString::number(not_found));
                LogHeadline(">> Failed    :", QString::number(failed));
            }
            else if (not_found == 0 && failed == 0 && copied > 0)
            {
                WorldBuildingModule::LogDebug(">> All Found : " + QString::number(copied).toStdString());
                LogHeadline(">> All Found :", QString::number(copied));
            }
            else if (not_found == 0 && failed == 0 && copied == 0)
            {
                WorldBuildingModule::LogDebug(">> There was no " + subfolder.toStdString() + " in this scene");
                Log(QString(">> There was no %1 in the scene").arg(subfolder));
            }
            if (replaced >= 0)
            {
                LogHeadline(">> Replaced material references:", QString::number(replaced));
            }
            
            // Remove the dir if its empty
            if (copied == 0)
            {
                store_location.cdUp();
                store_location.rmdir(subfolder);
            }
            else
                store_location.cdUp();
        }
        else
            WorldBuildingModule::LogError(">> Failed to create " + subfolder.toStdString() + " folder, aborting processing");

        LogLineEnd();
        return ref_to_file;
    }

    int SceneExporter::ReplaceMaterialTextureRefs(const QDir &store_location_textures, const QString &asset_base_url, const QString &material_file_path)
    {
        QString KEYWORD = "texture ";

        QHash<QString, QString> old_to_new_ref;
        QFile material_file(material_file_path);
        if (material_file.open(QIODevice::ReadWrite|QIODevice::Text))
        {
            QByteArray content = material_file.readAll();
            content.replace('\t', " ");
            content = content.trimmed();

            QString replace_url = "";
            QString texture_ref = "";

            int i_start = content.indexOf(KEYWORD);
            while (i_start != -1)
            {
                i_start += QString(KEYWORD).length();
                int i_end_space = content.indexOf(" ", i_start);
                int i_end_endl = content.indexOf('\n', i_start);

                // If either was not found
                if (i_end_space == -1 && i_end_endl == -1)
                {
                    i_start = content.indexOf(KEYWORD, i_start);
                    continue;
                }

                // Make the ' ' or '\n' the end index, which one is lower
                int i_end = i_end_endl;
                if (i_end_space > 0)
                    if (i_end_space < i_end)
                        i_end = i_end_space;
                if (i_end != -1)
                {
                    int ref_len = i_end - i_start;
                    QByteArray texture_ref_b = content.mid(i_start, ref_len);
                    texture_ref = QString(texture_ref_b);

                    // Try to get the texture from texture cache and store it to our texture location with qt or ogre
                    // depending on the pixel format and components
                    replace_url = TryStoreTexture(texture_ref, store_location_textures, asset_base_url);
                    if (!replace_url.isEmpty() && replace_url != "DYNAMIC_TEXTURE_CREATION_FAILED")
                        old_to_new_ref[texture_ref] = replace_url;
                }

                // Look for the next texture
                i_start = content.indexOf(KEYWORD, i_start);
            }

            // If the texture could be replicated to our export location, replace the old refs with our new url ref
            if (old_to_new_ref.count() > 0)
            {
                foreach(QString old_ref, old_to_new_ref.keys())
                {
                    QByteArray new_ref(old_to_new_ref[old_ref].toStdString().c_str());
                    content.replace(QByteArray(old_ref.toStdString().c_str()), new_ref);
                }
                material_file.resize(0);
                material_file.write(content);
            }
            material_file.close();
        }
        else
            WorldBuildingModule::LogDebug("SceneExport: Could not open copied material script");

        return old_to_new_ref.count();
    }

    int SceneExporter::ReplaceParticleMaterialRefs(const QDir store_location_particles, const QString &asset_base_url, const QString &particle_file_path)
    {
        bool overwrite = false;
        int replaced = 0;
        QString old_ref;
        QString new_ref;

        QDir texture_location(store_location_particles);
        texture_location.cdUp();
        texture_location.cd("textures");

        QFile particle_file(particle_file_path);
        if (particle_file.open(QIODevice::ReadWrite|QIODevice::Text))
        {
            QByteArray content = particle_file.readAll();
            content.replace('\t', " ");
            content = content.trimmed();

            QString replace_url = "";
            QString texture_ref = "";

            int i_start = content.indexOf("material ");
            if (i_start != -1)
            {
                i_start += QString("material ").length();
                int i_end_space = content.indexOf(" ", i_start);
                int i_end_endl = content.indexOf('\n', i_start);

                // If either was not found
                if (i_end_space == -1 && i_end_endl == -1)
                    return 0;

                // Make the ' ' or '\n' the end index, which one is lower
                int i_end = i_end_endl;
                if (i_end_space > 0)
                    if (i_end_space < i_end)
                        i_end = i_end_space;
                if (i_end != -1)
                {
                    int ref_len = i_end - i_start;
                    QByteArray material_ref_b = content.mid(i_start, ref_len);
                    old_ref = QString(material_ref_b);
                    

                    QString cache_file = GetCacheFilename(old_ref, "MaterialScript");
                    QString file_only = cache_file.split("/").last().toLower();
                    file_only.replace(".materialscript", ".material");


                    // 1. Check from our store location if this material script was already
                    //    moved and its textures replaced inside. This is the ideal case
                    if (store_location_particles.exists(file_only))
                    {
                        new_ref = asset_base_url + "/scripts/" + file_only;
                        overwrite = true;
                    }
                    // 2. Check the cache if this material is in there. If found
                    //    we have to replace the textures and move it to our scripts location
                    else
                    {
                        if (QFile::exists(cache_file))
                        {
                            QString new_material_path = store_location_particles.absoluteFilePath(file_only);
                            if (QFile::copy(cache_file, new_material_path))
                            {
                                new_ref = asset_base_url + "/scripts/" + file_only;
                                overwrite = true;

                                ReplaceMaterialTextureRefs(texture_location, asset_base_url, new_material_path);
                            }
                        }
                        // 3. Check if this "material" is really a texture. There is a mechanish in naali and the legacy
                        //    viewer to accept textures here too. It will generate a fullbringht material with that texture dynamically.
                        else
                        {
                            QString replace_url = TryStoreTexture(old_ref, texture_location, asset_base_url);
                            if (!replace_url.isEmpty() && replace_url != "DYNAMIC_TEXTURE_CREATION_FAILED")
                            {
                                new_ref = replace_url;
                                overwrite = true;
                            }
                        }
                    }
                }
            }

            if (overwrite)
            {
                content.replace(QByteArray(old_ref.toStdString().c_str()), QByteArray(new_ref.toStdString().c_str()));
                particle_file.resize(0);
                particle_file.write(content);
                replaced++;
            }
            particle_file.close();
        }

        return replaced;
    }

    QString SceneExporter::TryStoreTexture(const QString &original_ref, const QDir &store_location, const QString &base_url)
    {
        QString new_ref = "DYNAMIC_TEXTURE_CREATION_FAILED"; // a bit of a hack, yeah
        Foundation::TextureServiceInterface *texture_service = framework_->GetService<Foundation::TextureServiceInterface>();
        if (!texture_service)
            return new_ref;

        TextureDecoder::TextureResource *texture = texture_service->GetFromCache(original_ref.toStdString());
        if (texture)
        {
            // This will get the original name of a web texture <name>.png
            // and for UUIDs/other single word id's it will fallback to <UUID>.png
            QString file_only = QString(texture->GetId().c_str()).split("/").last();
            file_only = file_only.split(".").first();
            if (file_only.isEmpty())
                file_only = QString(texture->GetId().c_str()).split(".").first();
            file_only.append(".png");

            // New filename in our export location and the url to replace in the material script
            QString tex_filename = store_location.absoluteFilePath(file_only);
            QString replace_url = base_url + "/textures/" + file_only;

            // Read texture resource data
            int ogre_format = texture->GetFormat();
            int comps = texture->GetComponents();

            // -1 means jpeg2000, lets use ogre to get the correct pixelformat
            // as qt does not provide ABGR directly
            if (ogre_format == -1)
            {
                Ogre::Image image;
                Ogre::PixelFormat ogre_image_format;
                if (comps == 1)
                    ogre_image_format = Ogre::PF_L8;
                else if (comps == 2)
                    ogre_image_format = Ogre::PF_BYTE_LA;
                else if (comps == 3)
                    ogre_image_format = Ogre::PF_B8G8R8;
                else if (comps == 4)
                    ogre_image_format = Ogre::PF_A8B8G8R8;
                else
                {
                    WorldBuildingModule::LogDebug(">> Failed to store material texture with ogre, unhandled comps: " + QString::number(comps).toStdString());
                    return new_ref;
                }                
                image.loadDynamicImage(texture->GetData(), texture->GetWidth(), texture->GetHeight(), ogre_image_format);
                image.save(tex_filename.toStdString());                           
                new_ref = replace_url;
            }
            else
            {
                // Try to resolce qt format with ogre format
                QImage::Format qt_format;
                switch (ogre_format)
                {
                    case 6:
                        qt_format = QImage::Format_RGB16;
                        break;
                    case 26:
                        qt_format = QImage::Format_RGB32;
                        break;
                    case 12:
                        qt_format = QImage::Format_ARGB32;
                        break;
                    default:
                        qt_format = QImage::Format_Invalid;
                        break;
                }

                // If qt format can be used, lets make a qimage out of the data
                if (qt_format != QImage::Format_Invalid)
                {
                    QImage image(texture->GetData(), texture->GetWidth(), texture->GetHeight(), qt_format);
                    if (!image.isNull())
                    {
                        if (image.save(tex_filename, "PNG"))
                            new_ref = replace_url;
                        else
                            WorldBuildingModule::LogDebug(">> Qt failed to store texture as png, skipping texture");
                    }
                    else
                        WorldBuildingModule::LogDebug(">> Failed to create image from raw texture data, skipping texture");
                }
                else
                    WorldBuildingModule::LogDebug(">> Could not resolve texture format: " + QString::number(ogre_format).toStdString() + ", skipping texture");
            }
        }
        else
            return ""; // empty means was not in cache aka "not found"

        return new_ref;
    }

    void SceneExporter::CleanLocalDir(QDir dir)
    {
        int deleted = 0;
        QFileInfoList info_list = dir.entryInfoList(QDir::Files);
        foreach(QFileInfo file_info, info_list)
        {
            if (dir.remove(file_info.fileName()))
                deleted++;
        }
        Log(QString("Cleaned directory: %1 removed %2 files").arg(dir.absolutePath(), QString::number(deleted)));
    }

    QString SceneExporter::GetCacheFilename(const QString &asset_id, const QString &type)
    {
        std::string temp = asset_id.toStdString();
        QCryptographicHash md5_engine(QCryptographicHash::Md5);
        md5_engine.addData(temp.c_str(), temp.size());
        QString md5_hash(md5_engine.result().toHex());
        md5_engine.reset();

        QString cache_path = framework_->GetPlatform()->GetApplicationDataDirectory().c_str();
        cache_path.append("/assetcache/");
        cache_path.append(md5_hash);
        cache_path.append(".");
        cache_path.append(type);
        return cache_path;
    }
}
