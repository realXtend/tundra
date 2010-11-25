// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "Matrix4Decorator.h"

namespace PythonScript {
    //! Create a new matrix. This is initialised as an identity matrix
    Matrix4* Matrix4Decorator::new_Matrix4() { return new Matrix4(); }

    Vector3df Matrix4Decorator::getTranslation(const Matrix4 *self) const { return self->getTranslation(); }
    
    void Matrix4Decorator::setTranslation(Matrix4* self, const Vector3df& vec)
    {
        self->setTranslation(vec);
    }
}

