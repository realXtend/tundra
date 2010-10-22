// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "MeshFileRequest.h"

#include "MemoryLeakCheck.h"

namespace Library
{
    MeshFileRequest::MeshFileRequest(Vector3df ray_cast_pos) :
        ray_cast_pos_(ray_cast_pos)
    {
    }

    bool MeshFileRequest::AreMaterialsReady()
    {
        if (material_file_requests_.count() == material_ids_.count())
            return true;

        return false;
    }

    void MeshFileRequest::SetMaterialFileRequest(const request_tag_t tag, const QUrl url)
    {
        material_file_requests_.insert(tag, url);
    }

    void MeshFileRequest::SetMeshImageUrl(const QUrl url)
    {
        mesh_images_.append(url);
    }

    void MeshFileRequest::AddMaterialId(const QString mat_id)
    {
        material_ids_.append(mat_id);
    }

}
