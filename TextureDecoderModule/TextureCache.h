// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_TextureDecoder_TextureCache_h
#define incl_TextureDecoder_TextureCache_h

#include <QObject>
#include <QDir>

#include "Foundation.h"
#include "TextureResource.h"

namespace TextureDecoder
{
    //! Cache reply when texture is found,
    //! you can add tags here untill next TextureService 
    //! update call when its finally sent
    struct CacheReply
    {
        Foundation::ResourcePtr resource;
        RequestTagVector tags;   
    };

    class TextureCache : public QObject
    {
        Q_OBJECT

        public:
            TextureCache(Foundation::Framework* framework);
            virtual ~TextureCache();

        public slots:
            //! Store a texture to disk cache
            //! @param TextureInterface implementing pointer
            void StoreTexture(Foundation::TextureInterface *texture);

            //! Get a texture resource from cache
            //! @param texture id
            TextureResource *GetTexture(const std::string &texture_id);

            //! Read config
            void ReadConfig();

            //! Ui notifies this function that user changed settings
            void CacheConfigChanged(int cache_max_size, bool cache_everything);

            //! Clear the whole cache working directory
            void ClearCache();

            //! Getter for TextureService to know what to cache
            bool CacheEverything() { return cache_everything_; }

        private slots:
            //! Check cache and remove files until we are under the cache_max_size_
            //! @param make_extra_space - When checking cache after new stores we want to make some extra space
            //! so we dont have to clear again after the next store
            void CheckCacheSize(bool make_extra_space = true);

            //! Get has for filename
            QString GetHash(const std::string &id);

            //! Get full path of file to open it
            QString GetFullPath(QString hash_id);

        private:
            Foundation::Framework* framework_;

            QString DEFAULT_TEXTURE_CACHE_DIR;
            QString cache_path_;
            QDir cache_dir_;

            bool cache_everything_;
            int current_cache_size_;
            int cache_max_size_;
    };
}

#endif