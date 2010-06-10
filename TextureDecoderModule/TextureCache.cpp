// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "TextureCache.h"
#include "TextureDecoderModule.h"
#include "AssetServiceInterface.h"

#include "UiSettingsServiceInterface.h"

#include <QFile>
#include <QDataStream>
#include <QCryptographicHash>
#include <QSettings>

#include <QMessageBox>

namespace TextureDecoder
{
    TextureCache::TextureCache(Foundation::Framework* framework) :
        QObject(),
        framework_(framework),
        DEFAULT_TEXTURE_CACHE_DIR("texturecache"),
        cache_max_size_(0),
        cache_dir_(framework->GetPlatform()->GetApplicationDataDirectory().c_str()),
        cache_everything_(false),
        current_cache_size_(0)
    {
        // Set/init working directory
        if (!cache_dir_.exists(DEFAULT_TEXTURE_CACHE_DIR))
            cache_dir_.mkdir(DEFAULT_TEXTURE_CACHE_DIR);
        cache_dir_.cd(DEFAULT_TEXTURE_CACHE_DIR);

        // Read/init config file
        ReadConfig();

        // Connet to ui signals
        Foundation::UiSettingServicePtr ui_settings_service = framework_->GetServiceManager()->GetService<Foundation::UiSettingsServiceInterface>(Foundation::Service::ST_UiSettings).lock();
        if (ui_settings_service.get())
        {
            QObject *cache_widget = ui_settings_service->GetCacheSettingsWidget();
            if (cache_widget)
            {
                connect(cache_widget, SIGNAL(ClearTexturesPushed()), SLOT(ClearCache()));
                connect(cache_widget, SIGNAL(TextureSettingsSaved(int,bool)), SLOT(CacheConfigChanged(int,bool)));
            }
        }

        // Check the current cache size
        QFileInfoList file_info_list = cache_dir_.entryInfoList(QDir::Files);
        foreach(QFileInfo info, file_info_list)
            current_cache_size_ += info.size();
    }

    TextureCache::~TextureCache()
    {
    }

    void TextureCache::StoreTexture(Foundation::TextureInterface *texture)
    {
        QString id = GetHash(texture->GetId());
        QFile decoded_texture(GetFullPath(id));
        if (!decoded_texture.exists())
        {
            if (!decoded_texture.open(QIODevice::ReadWrite))
                return;

            // Write metadata
            QDataStream data_stream(&decoded_texture);
            data_stream << texture->GetComponents()
                        << texture->GetWidth()
                        << texture->GetHeight()
                        << texture->GetLevel()
                        << texture->GetFormat()
                        << (int)texture->GetDataSize();

            // Write data
            data_stream.writeRawData((const char *)texture->GetData(), texture->GetDataSize());
      
            decoded_texture.close();
            current_cache_size_ += decoded_texture.size();

            // Remove unneeded encoded asset cache entry for this texture
            boost::shared_ptr<Foundation::AssetServiceInterface> asset_service = framework_->GetServiceManager()->GetService<Foundation::AssetServiceInterface>(Foundation::Service::ST_Asset).lock();
            if (asset_service)
                asset_service->RemoveAssetFromCache(texture->GetId());

            TextureDecoderModule::LogDebug("Stored decoded texture " + id.left(7).toStdString() + "... to texture cache");
            CheckCacheSize();
        }
    }

    TextureResource *TextureCache::GetTexture(const std::string &texture_id)
    {
        QString id = GetHash(texture_id);
        QFile decoded_texture(GetFullPath(id));
        if (decoded_texture.exists())
        {
            if (!decoded_texture.open(QIODevice::ReadOnly))
                return 0;

            int data_length, format, level;
            uint components, width, height;

            // Read metadata
            QDataStream data_stream(&decoded_texture);
            data_stream >> components;
            data_stream >> width;
            data_stream >> height;
            data_stream >> level;
            data_stream >> format;
            data_stream >> data_length;

            // Init TextureResource with metadata
            TextureResource *texture = new TextureResource(texture_id, width, height, components);
            texture->SetLevel(level);
            texture->SetFormat(format);

            // Read data
            char *data_str_ptr = (char*)texture->GetData();
            data_stream.readRawData(data_str_ptr, data_length);

            decoded_texture.close();
            TextureDecoderModule::LogDebug("Found decoded texture " +id.left(7).toStdString() + "... from cache");
            return texture;
        }
        return 0;
    }

    void TextureCache::CheckCacheSize(bool make_extra_space)
    {
        if (cache_max_size_ == 0)
            return;

        // Remove files as long as we are under limit again
        if (current_cache_size_ > cache_max_size_)
        {
            int aimed_size = cache_max_size_;
            if (make_extra_space)
                aimed_size -= (5*1024*1024); // 5mb under max size
            if (aimed_size < 1)
                return;

            qint64 removed_bytes = 0;
            int removed_files = 0;

            QFileInfoList file_info_list = cache_dir_.entryInfoList(QDir::Files, QDir::Time | QDir::Reversed);
            foreach(QFileInfo info, file_info_list)
            {
                qint64 current_file_size = info.size();
                if (!cache_dir_.remove(info.fileName()))
                    continue;
                removed_files++;
                removed_bytes += current_file_size;
                current_cache_size_ -= current_file_size;
                if (current_cache_size_ < aimed_size)
                    break;
            }

            TextureDecoderModule::LogInfo("Texture cache was over limit. Removed " + QString::number(removed_files).toStdString() + 
                " files, total of " + QString::number(removed_bytes).toStdString() + " bytes");
        }
    }

    void TextureCache::ReadConfig()
    {
        QSettings cache_config(QSettings::IniFormat, QSettings::UserScope, "realXtend", "configuration/CacheSettings");
        cache_everything_ = cache_config.value("TextureCache/CacheEverything", QVariant(false)).toBool();
        cache_max_size_ = cache_config.value("TextureCache/MaxSize", QVariant(0)).toInt();
    }

    void TextureCache::CacheConfigChanged(int cache_max_size, bool cache_everything)
    {
        // Update local variables
        cache_everything_ = cache_everything;
        if (cache_max_size != cache_max_size_)
        {
            cache_max_size_ = cache_max_size;
            CheckCacheSize(false);
        }
    }

    void TextureCache::ClearCache()
    {
        QFileInfoList file_info_list = cache_dir_.entryInfoList(QDir::Files);
        if (file_info_list.count() > 0)
        {
            qint64 removed_bytes = 0;
            int removed_files = 0;
            foreach(QFileInfo info, file_info_list)
            {
                qint64 temp_size = info.size();
                if (cache_dir_.remove(info.fileName()))
                {
                    removed_files++;
                    removed_bytes += temp_size;
                }
            }

            // Notify user
            qreal removed_bytes_f = removed_bytes;
            QString mb_string = QString::number(((removed_bytes_f/1024)/1024));
            mb_string = mb_string.left(mb_string.indexOf(".")+3);
            QMessageBox::information(0, "Texture Cache", QString("Texture cache cleared, removed %1 files total of " + mb_string + " mb").arg(removed_files));

            // Reset tracking size
            current_cache_size_ = 0;
        }
        else
            QMessageBox::information(0, "Texture Cache", "There are currently no files in texture cache");
    }

    QString TextureCache::GetHash(const std::string &id)
    {
        QCryptographicHash md5_engine_(QCryptographicHash::Md5);
        md5_engine_.addData(id.c_str(), id.size());
        QString md5_hash(md5_engine_.result().toHex());
        md5_engine_.reset();
        return md5_hash;
    }

    QString TextureCache::GetFullPath(QString hash_id)
    {
        return QString(cache_dir_.absolutePath() + "/" + hash_id + ".decoded.Texture");
    }
}