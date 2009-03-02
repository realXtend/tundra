// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"

//! Unit test for framework
BOOST_AUTO_TEST_SUITE(test_suite_foundation)

BOOST_AUTO_TEST_CASE( framework_test_module )
{
    Foundation::Framework fw;
    fw.go();

//    Foundation::EntityPtr entity = fw.getEntityManager()->createEntity();
//    BOOST_REQUIRE (entity.get() != NULL);
}

BOOST_AUTO_TEST_SUITE_END()
