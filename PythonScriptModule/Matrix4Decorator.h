// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_PythonScriptModule_Matrix4Decorator_h
#define incl_PythonScriptModule_Matrix4Decorator_h

#include <PythonQt.h>
#include <QObject>
#include "Core.h"

namespace PythonScript
{
    class Matrix4Decorator : public QObject
    {    
        Q_OBJECT
        
        public slots:
            Matrix4* new_Matrix4();
            Vector3df getTranslation(const Matrix4* self) const;
            void setTranslation(Matrix4* self, const Vector3df& vec);
    };
}

#endif

