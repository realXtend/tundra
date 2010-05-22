// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "ResourceMetadataGetter.h"
#include "RexTypes.h"

#include <QNetworkRequest>
#include <QNetworkReply>

#include "OgreImageTextureResource.h"

namespace OgreRenderer
{
    ResourceMetadataGetter::ResourceMetadataGetter(Foundation::Framework *framework) :
        QObject(),
        framework_(framework),
        network_manager_(new QNetworkAccessManager(this))
    {
        connect(network_manager_, SIGNAL(finished(QNetworkReply*)), SLOT(MetadataFetched(QNetworkReply*)));
    }

    ResourceMetadataGetter::~ResourceMetadataGetter()
    {
    }

    void ResourceMetadataGetter::GetMetadata(QString id, QString type, request_tag_t tag)
    {
        ongoing_fetches_[id] = tag;
        network_manager_->get(QNetworkRequest(QUrl(id.replace("data", "metadata"))));
    }

    void ResourceMetadataGetter::MetadataFetched(QNetworkReply *reply)
    {
        QString id = reply->url().toString().replace("metadata", "data");
        if (ongoing_fetches_.contains(id))
        {
            request_tag_t tag = ongoing_fetches_[id];

            const QByteArray &inbound_metadata = reply->readAll();
            QString decoded_metadata = QString::fromUtf8(inbound_metadata.data());
            QString content_type("\"content_type\":");
            int index = decoded_metadata.indexOf(content_type);
            int start = decoded_metadata.indexOf("\"", index + content_type.length());
            start++;
            int end = decoded_metadata.indexOf("\"", start);
            content_type = decoded_metadata.mid(start, end-start);

            if (content_type == "image/jpg" || content_type == "image/jpeg")
            {
                emit Metadata(id.toStdString(), OgreImageTextureResource::GetTypeStatic(), tag);
            }
            ongoing_fetches_.remove(id);
        }
        reply->deleteLater();
    }
}