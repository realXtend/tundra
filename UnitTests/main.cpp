

#define BOOST_TEST_MAIN
#define BOOST_TEST_MODULE MyTest
#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"

BOOST_AUTO_TEST_CASE( framework_entity_test )
{
    Foundation::Framework fw;
    fw.go();

    Foundation::EntityPtr entity = fw.getEntityManager()->createEntity();
    BOOST_REQUIRE (entity.get() != NULL);
}

