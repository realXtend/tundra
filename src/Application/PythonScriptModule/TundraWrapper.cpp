// For conditions of distribution and use, see copyright notice in LICENSE

#include "StableHeaders.h"
#include "TundraWrapper.h"

#include "SceneAPI.h"
#include "Scene.h"
#include "Entity.h"

#include "AssetAPI.h"
#include "AssetReference.h"

namespace PythonScript
{
    // TundraInstanceDecorator

    QObject *TundraInstanceDecorator::CreateEntityLocalRaw(Scene *scene, const QStringList &components, AttributeChange::Type change, bool defaultNetworkSync)
    {
         return scene->CreateEntity(scene->NextFreeIdLocal(), components, change, defaultNetworkSync).get();
    }
    
    QObject* TundraInstanceDecorator::GetComponentRaw(Entity *entity, const QString &typeName)
    {
        ComponentPtr compSharedPtr = entity->GetComponent(typeName);
        return compSharedPtr.get();
    }

    QObject* TundraInstanceDecorator::GetOrCreateComponentRaw(Entity *entity, const QString &typeName)
    {
        ComponentPtr compSharedPtr = entity->GetOrCreateComponent(typeName);
        return compSharedPtr.get();
    }

  /*QObject* TundraInstanceDecorator::get(ScenePtr self)
    {
        return self.get();
	}
  */

    QObject *TundraInstanceDecorator::GetSceneRaw(SceneAPI *sceneapi, const QString &scenename)
    {
        return sceneapi->GetScene(scenename).get();
    }
    
    // TundraDecorator

    /*QObject* TundraDecorator::get(ScenePtr self)
    {
        return self.get();
    }*/

    AssetReference *TundraDecorator::new_AssetReference() 
    {
        return new AssetReference();
    }

    AssetReference *TundraDecorator::new_AssetReference(const QString &assetRef)
    {
        return new AssetReference(assetRef);
    }
    
    AssetReference *TundraDecorator::new_AssetReference(const QString &assetRef, const QString &assetType)
    {
        return new AssetReference(assetRef, assetType);
    }

    void TundraDecorator::delete_AssetReference(AssetReference* obj)
    {
        SAFE_DELETE(obj);
    }

    QString TundraDecorator::ref(const AssetReference* obj)
    {
        return obj->ref;
    }

    void TundraDecorator::setRef(AssetReference* obj, const QString &assetRef)
    {
        obj->ref = assetRef;
    }

    QString TundraDecorator::type(const AssetReference *obj)
    {
        return obj->type;
    }

    void TundraDecorator::setType(AssetReference *obj, const QString &assetType)
    {
        obj->type = assetType;
    }
  
    float TundraDecorator::x(float3* self)
    {
        return self->x;
    }
    
    float TundraDecorator::y(float3* self)
    {
        return self->y;
    }
    
    float TundraDecorator::z(float3* self)
    {
        return self->z;
    }

    //registering float3 meta type
    int id = qRegisterMetaType<float3>("float3");

    void TundraDecorator::setx(float3* self, float value)
    {
        self->x = value;
    }
    
    void TundraDecorator::sety(float3* self, float value)
    {
        self->y = value;
    }
    
    void TundraDecorator::setz(float3* self, float value)
    {
        self->z = value;
    }

    float3 TundraDecorator::neg(float3* self)
    {   
        return self->Neg();      
    }

    float3 TundraDecorator::position(const Transform* self)
    {
        return self->pos;
    }

    float3 TundraDecorator::rotation(const Transform* self)
    {
        return self->rot;
    }

    float3 TundraDecorator::scale(const Transform* self)
    {
        return self->scale;
    }

    float3 TundraDecorator::QuatToEulerZYX(Quat* self) const
    {
        return self->ToEulerZYX();
    }

    void TundraDecorator::set(float3* self, float x, float y, float z)
    {
        self->Set(x, y, z);
    }
  
    void TundraDecorator::SetPos(Transform* self, float x, float y, float z)
    {
        self->SetPos(x, y, z);
    }

    void TundraDecorator::SetRot(Transform* self, float x, float y, float z)
    {
        self->SetRotation(x, y, z);
    }

    void TundraDecorator::SetScale(Transform* self, float x, float y, float z)
    {
        self->SetScale(x, y, z);
    }

    //Quats
    int q_id = qRegisterMetaType<Quat>("Quat");

    Quat TundraDecorator::FromEulerZYX(Quat* self, float z, float y, float x) const
    {
        return self->FromEulerZYX(z, y ,x);
    }
    
    void TundraDecorator::SetFromAxisAngle(Quat* self, const float3 &rotationAxis, const float rotationAngleRadians)
    {
        return self->SetFromAxisAngle(rotationAxis, rotationAngleRadians);
    }
}
