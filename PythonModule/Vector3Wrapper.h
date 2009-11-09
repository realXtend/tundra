#ifndef incl_Vector3Wrapper_h
#define incl_Vector3Wrapper_h

#include "PythonQt.h"
#include <QObject>
#include "Foundation.h"

namespace PythonScript {
	class CustomObject {
		public:
		  CustomObject() {}
		  CustomObject(const QString& first, const QString& last) { _firstName = first; _lastName = last; }

		  QString _firstName;
		  QString _lastName;

	};

	class CustomObjectWrapper : public QObject {

		  Q_OBJECT

		public slots:
		  // add a constructor
		  CustomObject* new_CustomObject(const QString& first, const QString& last) { return new CustomObject(first, last); }

		  // add a destructor
		  void delete_CustomObject(CustomObject* o) { delete o; }

		  // add access methods
		  QString firstName(CustomObject* o) { return o->_firstName; }

		  QString lastName(CustomObject* o) { return o->_lastName; }

		  void setFirstName(CustomObject* o, const QString& name) { o->_firstName = name; }

		  void setLastName(CustomObject* o, const QString& name) { o->_lastName = name; }

	};

	class Vector3Wrapper : public QObject
	{
		Q_OBJECT
		Q_PROPERTY(float x READ GetX WRITE SetX)
		Q_PROPERTY(float y READ GetY WRITE SetY)
		Q_PROPERTY(float z READ GetZ WRITE SetZ)

		public slots:
			Core::Vector3df* new_Vector3(const float x, const float y, const float z) { return new Core::Vector3df(x, y, z); }

			float x(Core::Vector3df* vec) { return vec->x; }
			float y(Core::Vector3df* vec) { return vec->y; }
			float z(Core::Vector3df* vec) { return vec->z; }

			float GetX() { return 0; }
			float GetY() { return 0; }
			float GetZ() { return 0; }
			
			void SetX(float value) {}
			void SetY(float value) {}
			void SetZ(float value) {}
			//float getDistanceFrom(const Core::Vector3df* first, const Core::Vector3df* second) { first->getDistanceFrom(second); }

	};
}

#endif