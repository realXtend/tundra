#ifndef incl_QuaternionWrapper_h
#define incl_QuaternionWrapper_h

#include <PythonQt.h>
#include <QObject>
#include "Core.h"

namespace PythonScript {
   
	class QuaternionWrapper : public QObject
	{    
		Q_OBJECT
		
        public slots:
			Quaternion* new_Quaternion(const float x, const float y, const float z, const float w) { return new Quaternion(x, y, z, w); }
            Quaternion* new_Quaternion(const float x, const float y, const float z) { return new Quaternion(x, y, z); }
            Quaternion* new_Quaternion(const Vector3df& vec) { return new Quaternion(vec); }

			float x(Quaternion* self) { return self->x; }
			float y(Quaternion* self) { return self->y; }
			float z(Quaternion* self) { return self->z; } 
			float w(Quaternion* self) { return self->w; } 

			void setx(Quaternion* self, float value) { self->x = value; }
			void sety(Quaternion* self, float value) { self->y = value; }
			void setz(Quaternion* self, float value) { self->z = value; }
            void setw(Quaternion* self, float value) { self->w = value; }

            void toEuler(Quaternion* self, Vector3df& euler) { 
                self->toEuler(euler);
            }
	};
}

#endif