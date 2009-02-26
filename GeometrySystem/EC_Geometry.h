
#ifndef __incl_GeometrySystem_EC_Geometry_h__
#define __incl_GeometrySystem_EC_Geometry_h__

#include "ComponentInterface.h"
#include "Foundation.h"
#include <Poco/Delegate.h>

namespace Geometry
{
    //! Geometry component. Stores position, orientation and scale
    class EC_Geometry : public Foundation::ComponentInterface
    {
        DECLARE_EC(EC_Geometry);
    public:
        virtual ~EC_Geometry() {}

        EC_Geometry &operator =(const EC_Geometry &other)
        {
            if (this != &other)
            {
                mPosition = other.mPosition;
                mScale = other.mScale;
                mOrientation = other.mOrientation;
            }

            return *this;
        }

    private:
        EC_Geometry() { }
        EC_Geometry(const EC_Geometry &other) { mPosition = other.mPosition; mScale = other.mScale; mOrientation = other.mOrientation; }

        Core::Vector3 mPosition;
        Core::Vector3 mScale;
        Core::Quaternion mOrientation;
    };
}

#endif
