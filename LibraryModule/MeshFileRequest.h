/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   MeshFileRequest.h
 *  @brief  
 */

#ifndef incl_MeshFileRequest_h
#define incl_MeshFileRequest_h

#include <QUrl>

namespace Library
{
    class MeshFileRequest
    {

    public:
        //! constructor
        MeshFileRequest(const Vector3df ray_cast_pos);
        
        QMap<request_tag_t, QUrl> GetMaterialFileRequests() { return material_file_requests_; };
        Vector3df GetRayCastPos() { return ray_cast_pos_; };
        QUrl GetMeshFileUrl() { return mesh_file_url_; };
        QList<QString> GetMaterialIds() { return material_ids_; };
        QList<QUrl> GetMeshImages() { return mesh_images_; };
        request_tag_t GetMeshRequestTag() { return mesh_request_tag_; };
        QString GetRootUrl();
        
        void SetMeshFileUrl(const QUrl url) { mesh_file_url_ = url; };
        void SetMaterialFileRequest(const request_tag_t tag, const QUrl url); 
        void SetMeshImageUrl(const QUrl url); 
        void SetMeshRequestTag(const request_tag_t tag) { mesh_request_tag_ = tag; };

        bool AreMaterialsReady();
        void AddMaterialId(const QString mat_id);
    private:
        
        QUrl mesh_file_url_;
        Vector3df ray_cast_pos_;
        QMap<request_tag_t, QUrl> material_file_requests_;
        QList<QString> material_ids_;
        QList<QUrl> mesh_images_;
        request_tag_t mesh_request_tag_;

    };
}

#endif
