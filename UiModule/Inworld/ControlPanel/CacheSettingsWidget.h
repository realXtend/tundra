// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_CacheSettingsWidget_h
#define incl_UiModule_CacheSettingsWidget_h

#include "ui_CacheSettingsWidget.h"
#include "UiModuleApi.h"

namespace CoreUi
{
    class UI_MODULE_API CacheSettingsWidget : public QWidget, public Ui::CacheSettingsWidget
    {

    Q_OBJECT

    public:
        CacheSettingsWidget(QObject *settings_object);
        virtual ~CacheSettingsWidget();

    public slots:
        void SetAssetConfigValues(int max_cache_size);
        void SetTextureConfigValues(int max_cache_size, bool cache_everything);

    private slots:
        void ReadConfig();
        void SettingsSaved();
        void ClearAssets();
        void ClearTextures();
    private:
        bool current_textures_cache_everything_;
        int current_textures_max_size_;
        int current_assets_max_size_;

    signals:
        void ClearAssetsPushed();
        void ClearTexturesPushed();
        void AssetSettingsSaved(int new_max_cache_size);
        void TextureSettingsSaved(int new_max_cache_size, bool cache_everything);

    };
}

#endif