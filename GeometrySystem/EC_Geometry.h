// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_GeometrySystem_EC_Geometry_h
#define incl_GeometrySystem_EC_Geometry_h

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
                position_ = other.position_;
                scale_ = other.scale_;
                orientation_ = other.orientation_;
            }

            return *this;
        }

    private:
        EC_Geometry() { }
        EC_Geometry(const EC_Geometry &other) { position_ = other.position_; scale_ = other.scale_; orientation_ = other.orientation_; }

        Core::Vector3df position_;
        Core::Vector3df scale_;
        Core::Quaternion orientation_;
    };
}

#endif
