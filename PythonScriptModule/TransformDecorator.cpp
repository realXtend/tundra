// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "TransformDecorator.h"

namespace PythonScript {

	Transform* TransformDecorator::new_Transform(const Vector3df &pos, const Vector3df &rot, const Vector3df &scale) 
	{
		return new Transform(pos, rot, scale);
	}
	
	Transform* TransformDecorator::new_Transform() 
	{
		return new Transform();
	}
    
	void TransformDecorator::delete_Transform(Transform* self)
	{
		delete self;
	}

	Vector3df TransformDecorator::position(const Transform* self)
	{
		return self->position;
	}

	Vector3df TransformDecorator::rotation(const Transform* self)
	{
		return self->rotation;
	}

	Vector3df TransformDecorator::scale(const Transform* self)
	{
		return self->scale;
	}

	void TransformDecorator::SetPos(Transform* self, float x, float y, float z) 
	{
		self->SetPos(x, y, z);
	}

	void TransformDecorator::SetRot(Transform* self, float x, float y, float z) 
	{
		self->SetRot(x, y, z);
	}

	void TransformDecorator::SetScale(Transform* self, float x, float y, float z) 
	{
		self->SetScale(x, y, z);
	}
    
}
