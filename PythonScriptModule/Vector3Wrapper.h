#ifndef incl_Vector3Wrapper_h
#define incl_Vector3Wrapper_h

#include <PythonQt.h>
#include <QObject>
#include "Core.h"

namespace PythonScript {
   
	class Vector3Wrapper : public QObject
	{    
		Q_OBJECT
		
        public slots:
			Vector3df* new_Vector3df(const float x, const float y, const float z) { return new Vector3df(x, y, z); }
            Vector3df* new_Vector3df() { return new Vector3df(); }

			float x(Vector3df* self) { return self->x; }
			float y(Vector3df* self) { return self->y; }
			float z(Vector3df* self) { return self->z; } 
			
			void setx(Vector3df* self, float value) { self->x = value; }
			void sety(Vector3df* self, float value) { self->y = value; }
			void setz(Vector3df* self, float value) { self->z = value; }
			float getDistanceFrom(const Vector3df* first, const Vector3df& second) { return first->getDistanceFrom(second); }
	};
}

#endif