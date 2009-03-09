// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"
#include "ServiceInterfaces.h"
#include "StaticModuleDefinitions.h"

//! Unit test for framework
BOOST_AUTO_TEST_SUITE(test_suite_foundation)

BOOST_AUTO_TEST_CASE( framework_test_module )
{
    Foundation::Framework fw;
    
    Test::StaticModuleDefinitions static_test;
    static_test(&fw);


    fw.Go();
}

void frameworkConfigurationManagerTest()
{
    Foundation::ConfigurationManager manager("./testing/configuration.xml");
    BOOST_CHECK_EQUAL (manager.DeclareSetting("testGroup1", "test_key1", "default_value"), std::string("test_string"));
    BOOST_CHECK_EQUAL (manager.DeclareSetting("testGroup1", "test_key1", std::string("default_value")), std::string("test_string"));
    BOOST_CHECK_EQUAL (manager.DeclareSetting("testGroup2", "test_key2", 0), 123456);
    BOOST_CHECK_EQUAL (manager.DeclareSetting("testGroup2", "test_key3", false), true);
    BOOST_CHECK (Core::equals(manager.DeclareSetting("testGroup2", "test_key4", 0.f), 3.2f));
    BOOST_CHECK (manager.HasKey("testGroup1", "test_key1"));
    BOOST_CHECK (manager.HasKey("NoGroup", "no_key") == false);
}

BOOST_AUTO_TEST_CASE( framework_configuration_manager )
{
    frameworkConfigurationManagerTest();

#ifdef _DEBUG
     // twice, to check config file gets properly written in debug mode
    frameworkConfigurationManagerTest();
#endif
}

BOOST_AUTO_TEST_CASE( framework_platform )
{
    std::string appData = Foundation::Platform::GetApplicationDataDirectory();
    BOOST_CHECK (appData.find(Foundation::Application::Name()) != std::string::npos);

    std::wstring appData_unicode = Foundation::Platform::GetApplicationDataDirectoryW();
    BOOST_CHECK (appData_unicode.find(Foundation::Application::NameW()) != std::wstring::npos);

    appData = Foundation::Platform::GetUserDocumentsDirectory();
    appData_unicode = Foundation::Platform::GetUserDocumentsDirectoryW();
}

BOOST_AUTO_TEST_SUITE_END()
