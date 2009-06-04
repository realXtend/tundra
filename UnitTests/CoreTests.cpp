// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"

BOOST_AUTO_TEST_SUITE(core)


BOOST_AUTO_TEST_CASE( exception )
{
    Core::Exception e;
    try
    {
        e = Core::Exception("exception test");
        throw e;
        BOOST_CHECK(false);
    } catch (Core::Exception e)
    {
        BOOST_CHECK_EQUAL(std::string(e.what()), "exception test");
    }
}

BOOST_AUTO_TEST_CASE( vector3 )
{
    Core::Vector3df test;
    BOOST_CHECK_EQUAL (test, Core::Vector3df(0, 0, 0));

    test = Core::Vector3df(1, 1, 1);
    BOOST_CHECK_EQUAL (test, Core::Vector3D<Core::f32>::UNIT_SCALE_X);

    test = Core::Vector3df(1, 0, 0);
    BOOST_CHECK_EQUAL (test, Core::Vector3D<Core::f32>::UNIT_X);
    BOOST_CHECK_EQUAL (test.getLength(), 1.f);

    test = -test;
    BOOST_CHECK_EQUAL (test, Core::Vector3D<Core::f32>::NEGATIVE_UNIT_X);

    test *= 5.f;
    test.invert();
    BOOST_CHECK_EQUAL (test, Core::Vector3df(5, 0, 0));
    BOOST_CHECK_EQUAL (test.getLengthSQ(), 25.f);

    test.normalize();
    BOOST_CHECK_EQUAL (test.getLength(), 1.f);

    test.set(2, 0, 0);
    Core::Vector3df other;
    BOOST_CHECK_EQUAL (test.getDistanceFrom(other), 2.f);
    BOOST_CHECK_EQUAL (test.getDistanceFromSQ(other), 4.f);

    other.set(2, 2, 0);
    BOOST_CHECK_EQUAL (test.crossProduct(other), Core::Vector3df(0, 0, 4));
    BOOST_CHECK_EQUAL (test.dotProduct(other), 4.f);

    other = test * Core::Vector3D<Core::f32>::UNIT_SCALE_X;
    BOOST_CHECK_EQUAL (test, other);

    test /= 2.f;
    BOOST_CHECK_EQUAL (test, Core::Vector3D<Core::f32>::UNIT_X);

    Core::Vector3di test_str;
    test_str.set(2, 3, 4);
    std::stringstream ss;
    ss << test_str;
    Core::Vector3di other_str(1, 1, 1);
    ss >> other_str;
    BOOST_CHECK_EQUAL (test_str, other_str);

    Core::Vector3df test_fv(1.5f ,0.5f, 1.5f);
    Core::Vector3df other_fv;
    ss << test_fv;
    ss >> test_fv;
    BOOST_CHECK (Core::equals(test_fv.x, other_fv.x) && Core::equals(test_fv.y, other_fv.y) && Core::equals(test_fv.z, other_fv.z));

}

BOOST_AUTO_TEST_CASE( quaternion )
{
    Core::Quaternion test;
    BOOST_CHECK_EQUAL (test, Core::Quaternion::IDENTITY);

    test.set(1, 2, 3, 1);
    test.normalize();
    Core::Quaternion other = test * Core::Quaternion::IDENTITY;
    BOOST_CHECK_EQUAL (test, other);

    test.makeIdentity();
    BOOST_CHECK_EQUAL (test, Core::Quaternion::IDENTITY);

    test = Core::Quaternion(0, 0, 0, 0);
    BOOST_CHECK_EQUAL (test, Core::Quaternion::ZERO);

    test.makeIdentity();
    test *= 10.f;
    BOOST_CHECK_EQUAL (test, Core::Quaternion(0, 0, 0, 10.f));

    test.makeIdentity();
    Core::Vector3df testVector(1, 2, 3);
    testVector = test * testVector;
    BOOST_CHECK_EQUAL (testVector, Core::Vector3df(1, 2, 3));

    test = Core::Quaternion(-Core::PI, 0, 0);
    BOOST_CHECK_EQUAL (test, Core::Quaternion(-1, 0, 0, 0));
    test = Core::Quaternion(0, Core::HALF_PI, 0);
    BOOST_CHECK_EQUAL (test, Core::Quaternion(0, 0.70710683f, 0, 0.70710683f));
}

BOOST_AUTO_TEST_CASE( string_utils )
{
    BOOST_CHECK_EQUAL (Core::ToString(123),  std::string("123"));

    BOOST_CHECK (Core::ToWString("test") == std::wstring(L"test"));

    BOOST_CHECK_EQUAL (Core::ParseString<unsigned int>("543210"),  543210);
    BOOST_CHECK_EQUAL (Core::ParseString<int>("-1"),  -1);
}

BOOST_AUTO_TEST_SUITE_END()
