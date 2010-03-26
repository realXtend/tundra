// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"

BOOST_AUTO_TEST_SUITE(core)


BOOST_AUTO_TEST_CASE( exception )
{
    Exception e;
    try
    {
        e = Exception("exception test");
        throw e;
        BOOST_CHECK(false);
    } catch (Exception e)
    {
        BOOST_CHECK_EQUAL(std::string(e.what()), "exception test");
    }
}

BOOST_AUTO_TEST_CASE( vector3 )
{
    Vector3df test;
    BOOST_CHECK_EQUAL (test, Vector3df(0, 0, 0));

    test = Vector3df(1, 1, 1);
    BOOST_CHECK_EQUAL (test, Vector3D<f32>::UNIT_SCALE_X);

    test = Vector3df(1, 0, 0);
    BOOST_CHECK_EQUAL (test, Vector3D<f32>::UNIT_X);
    BOOST_CHECK_EQUAL (test.getLength(), 1.f);

    test = -test;
    BOOST_CHECK_EQUAL (test, Vector3D<f32>::NEGATIVE_UNIT_X);

    test *= 5.f;
    test.invert();
    BOOST_CHECK_EQUAL (test, Vector3df(5, 0, 0));
    BOOST_CHECK_EQUAL (test.getLengthSQ(), 25.f);

    test.normalize();
    BOOST_CHECK_EQUAL (test.getLength(), 1.f);

    test.set(2, 0, 0);
    Vector3df other;
    BOOST_CHECK_EQUAL (test.getDistanceFrom(other), 2.f);
    BOOST_CHECK_EQUAL (test.getDistanceFromSQ(other), 4.f);

    other.set(2, 2, 0);
    BOOST_CHECK_EQUAL (test.crossProduct(other), Vector3df(0, 0, 4));
    BOOST_CHECK_EQUAL (test.dotProduct(other), 4.f);

    other = test * Vector3D<f32>::UNIT_SCALE_X;
    BOOST_CHECK_EQUAL (test, other);

    test /= 2.f;
    BOOST_CHECK_EQUAL (test, Vector3D<f32>::UNIT_X);

    Vector3di test_str;
    test_str.set(2, 3, 4);
    std::stringstream ss;
    ss << test_str;
    Vector3di other_str(1, 1, 1);
    ss >> other_str;
    BOOST_CHECK_EQUAL (test_str, other_str);
    ss.clear();

    Vector3df test_fv(1.5f, 0.5f, 1.5f);
    Vector3df other_fv(1.5f, 0.5f, 1.5f);
    ss << test_fv;
    ss >> test_fv;
    BOOST_CHECK (equals(test_fv.x, other_fv.x) && equals(test_fv.y, other_fv.y) && equals(test_fv.z, other_fv.z));

    test_fv.set(0, 0, 0);
    other_fv.set(0, 0, 1);
    std::string test_fv_str = ToString(test_fv);
    other_fv = ParseString<Vector3df>(test_fv_str);
    BOOST_CHECK (equals(test_fv.x, other_fv.x) && equals(test_fv.y, other_fv.y) && equals(test_fv.z, other_fv.z));
}

BOOST_AUTO_TEST_CASE( quaternion )
{
    Quaternion test;
    BOOST_CHECK_EQUAL (test, Quaternion::IDENTITY);

    test.set(1, 2, 3, 1);
    test.normalize();
    Quaternion other = test * Quaternion::IDENTITY;
    BOOST_CHECK_EQUAL (test, other);

    test.makeIdentity();
    BOOST_CHECK_EQUAL (test, Quaternion::IDENTITY);

    test = Quaternion(0, 0, 0, 0);
    BOOST_CHECK_EQUAL (test, Quaternion::ZERO);

    test.makeIdentity();
    test *= 10.f;
    BOOST_CHECK_EQUAL (test, Quaternion(0, 0, 0, 10.f));

    test.makeIdentity();
    Vector3df testVector(1, 2, 3);
    testVector = test * testVector;
    BOOST_CHECK_EQUAL (testVector, Vector3df(1, 2, 3));

    test = Quaternion(-PI, 0, 0);
    BOOST_CHECK_EQUAL (test, Quaternion(-1, 0, 0, 0));
    test = Quaternion(0, HALF_PI, 0);
    BOOST_CHECK_EQUAL (test, Quaternion(0, 0.70710683f, 0, 0.70710683f));
}

BOOST_AUTO_TEST_CASE( string_utils )
{
    BOOST_CHECK_EQUAL (ToString(123),  std::string("123"));

    BOOST_CHECK (ToWString("test") == std::wstring(L"test"));

    BOOST_CHECK_EQUAL (ParseString<unsigned int>("543210"),  543210);
    BOOST_CHECK_EQUAL (ParseString<int>("-1"),  -1);
}

BOOST_AUTO_TEST_SUITE_END()
