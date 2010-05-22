// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_ResourceMetadataGetter_h
#define incl_OgreRenderer_ResourceMetadataGetter_h

#include "Foundation.h"

#include <QObject>
#include <QNetworkAccessManager>
#include <QMap>
#include <QPair>
#include <QUrl>

namespace OgreRenderer
{
    class ResourceMetadataGetter : public QObject
    {
        
    Q_OBJECT

    public:
        ResourceMetadataGetter(Foundation::Framework *framework);
        virtual ~ResourceMetadataGetter();

    public slots:
        void GetMetadata(QString id, QString type, request_tag_t tag);
        void MetadataFetched(QNetworkReply *reply);

    private:
        Foundation::Framework *framework_;
        QNetworkAccessManager *network_manager_;

        QMap<QString,request_tag_t> ongoing_fetches_;

    signals:
        void Metadata(std::string id, std::string type, request_tag_t tag);
    };
}

#endif