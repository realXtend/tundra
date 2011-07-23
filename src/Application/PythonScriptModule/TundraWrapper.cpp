
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
    
    // TundraDecorator

    AssetReference *TundraDecorator::new_AssetReference() 
    {
        return new AssetReference();
    }

    AssetReference *TundraDecorator::new_AssetReference(const QString &assetRef) 
    {
        return new AssetReference(assetRef, AssetAPI::GetResourceTypeFromAssetRef(assetRef));
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
}
