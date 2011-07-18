// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"
#include "MemoryLeakCheck.h"
#include "AssetReferenceDecorator.h"

namespace PythonScript {

    AssetReference* AssetReferenceDecorator::new_AssetReference(const QString &reference) 
    {
        return new AssetReference(reference);
    }

    AssetReference* AssetReferenceDecorator::new_AssetReference() 
    {
        return new AssetReference();
    }

    void AssetReferenceDecorator::delete_AssetReference(AssetReference* self)
    {
        delete self;
    }

    QString AssetReferenceDecorator::Ref(const AssetReference* self)
    {
        return self->ref;
    }

    void AssetReferenceDecorator::SetRef(AssetReference* self, QString newref)
    {
        self->ref = newref;
    }
}
