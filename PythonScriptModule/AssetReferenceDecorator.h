// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptModule_AssetReferenceDecorator_h
#define incl_PythonScriptModule_AssetReferenceDecorator_h

#include <QObject>
#include "AssetReference.h"

namespace PythonScript
{
    class AssetReferenceDecorator : public QObject
    {
        Q_OBJECT
	//apparently not possible in these decorators(?): Q_PROPERTY(QString ref READ GetRef WRITE SetRef)

    public slots:
        AssetReference* new_AssetReference(const QString &reference);
        AssetReference* new_AssetReference();
        void delete_AssetReference(AssetReference* self);

        QString Ref(const AssetReference* self);
        void SetRef(AssetReference* self, QString newref);
    };
}

#endif
