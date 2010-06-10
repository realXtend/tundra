// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "CacheSettingsWidget.h"

namespace CoreUi
{
    CacheSettingsWidget::CacheSettingsWidget(QObject *settings_object) :
        QWidget(0, Qt::Widget),
        current_textures_cache_everything_(false),
        current_textures_max_size_(0),
        current_assets_max_size_(0)
    {
        setupUi(this);
        ReadConfig();

        // Set read config values to ui
        SetTextureConfigValues(current_textures_max_size_, current_textures_cache_everything_);
        SetAssetConfigValues(current_assets_max_size_);

        connect(settings_object, SIGNAL(SaveSettingsClicked()),
                SLOT(SettingsSaved()));
        connect(pushButton_clear_assets, SIGNAL(clicked()),
                SLOT(ClearAssets()));
        connect(pushButton_clear_textures, SIGNAL(clicked()),
                SLOT(ClearTextures()));
    }

    CacheSettingsWidget::~CacheSettingsWidget()
    {
    }

    void CacheSettingsWidget::ReadConfig()
    {
        bool default_textures_cache_everything = false;
        int default_textures_max_size = 500*1024*1024;
        int default_assets_max_size = 300*1024*1024;

        // Init config file if file/segments doesent exist
        QSettings cache_config(QSettings::IniFormat, QSettings::UserScope, "realXtend", "configuration/CacheSettings");
        QString segment = "AssetCache";
        QFile config_file(cache_config.fileName());
        if (!config_file.exists())
        {
            cache_config.beginGroup("TextureCache");
            cache_config.setValue("CacheEverything", default_textures_cache_everything);
            cache_config.setValue("MaxSize", default_textures_max_size);
            cache_config.endGroup();

            cache_config.beginGroup("AssetCache");
            cache_config.setValue("MaxSize", default_assets_max_size);
            cache_config.endGroup();
            cache_config.sync();
        }
        else if (!cache_config.childGroups().contains("TextureCache"))
        {
            cache_config.beginGroup("TextureCache");
            cache_config.setValue("CacheEverything", default_textures_cache_everything);
            cache_config.setValue("MaxSize", default_textures_max_size);
            cache_config.endGroup();
            cache_config.sync();
        }
        else if (!cache_config.childGroups().contains("AssetCache"))
        {
            cache_config.beginGroup("AssetCache");
            cache_config.setValue("MaxSize", default_assets_max_size);
            cache_config.endGroup();
            cache_config.sync();
        }

        // Read config and update ui with them
        // Note: The key-value pairs might not be there even if the segment exists, lets do one more set of checks
        QVariant val;
        segment = "TextureCache/CacheEverything";
        val = cache_config.value(segment);
        if (val.isNull())
        {
            cache_config.setValue(segment, default_textures_cache_everything);
            current_textures_cache_everything_ = default_textures_cache_everything;
        }
        else
            current_textures_cache_everything_ = val.toBool();

        segment = "TextureCache/MaxSize";
        val = cache_config.value(segment);
        if (val.isNull())
        {
            cache_config.setValue(segment, default_textures_max_size);
            current_textures_max_size_ = default_textures_max_size;
        }
        else
            current_textures_max_size_ = val.toInt();
        
        segment = "AssetCache/MaxSize";
        val = cache_config.value(segment);
        if (val.isNull())
        {
            cache_config.setValue(segment, default_assets_max_size);
            current_assets_max_size_ = default_assets_max_size;
        }
        else
            current_assets_max_size_ = val.toInt();

        cache_config.sync();
    }

    void CacheSettingsWidget::SetAssetConfigValues(int max_cache_size)
    {
        spinBox_assets->setValue((max_cache_size/1024)/1024);
    }

    void CacheSettingsWidget::SetTextureConfigValues(int max_cache_size, bool cache_everything)
    {
        spinBox_textures->setValue((max_cache_size/1024)/1024);
        checkBox_decode_only_jpeg2000->setChecked(!cache_everything);
    }

    void CacheSettingsWidget::SettingsSaved()
    {
        // Update asset config
        int new_asset_cache_limit = spinBox_assets->value()*1024*1024;
        if (new_asset_cache_limit != current_assets_max_size_)
        {
            current_assets_max_size_ = new_asset_cache_limit;
            QSettings cache_config(QSettings::IniFormat, QSettings::UserScope, "realXtend", "configuration/CacheSettings");
            cache_config.beginGroup("AssetCache");
            cache_config.setValue("MaxSize", current_assets_max_size_);
            cache_config.endGroup();
            cache_config.sync();

            // Emit new asset cache values
            emit AssetSettingsSaved(current_assets_max_size_);
        }
        
        // Update texture config
        int new_texture_cache_limit = spinBox_textures->value()*1024*1024;
        bool new_cache_evertything = !checkBox_decode_only_jpeg2000->isChecked();
        if (new_cache_evertything != current_textures_cache_everything_ || new_texture_cache_limit != current_textures_max_size_)
        {
            current_textures_cache_everything_ = new_cache_evertything;
            current_textures_max_size_ = new_texture_cache_limit;

            QSettings cache_config(QSettings::IniFormat, QSettings::UserScope, "realXtend", "configuration/CacheSettings");
            cache_config.beginGroup("TextureCache");
            cache_config.setValue("CacheEverything", current_textures_cache_everything_);
            cache_config.setValue("MaxSize", current_textures_max_size_);
            cache_config.endGroup();
            cache_config.sync();
    
            // Emit new texture cache values
            emit TextureSettingsSaved(current_textures_max_size_, current_textures_cache_everything_);
        }
    }

    void CacheSettingsWidget::ClearAssets()
    {
        emit ClearAssetsPushed();
    }

    void CacheSettingsWidget::ClearTextures()
    {
        emit ClearTexturesPushed();
    }
}