// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"
#include "ServiceInterfaces.h"
#include "StaticModuleDefinitions.h"
#include "EC_Dummy.h"

//! Unit test for framework
BOOST_AUTO_TEST_SUITE(test_suite_foundation)

BOOST_AUTO_TEST_CASE( framework_platform )
{
    Foundation::Framework fw;

    std::string appData = fw.GetPlatform()->GetApplicationDataDirectory();
    BOOST_CHECK (appData.find(fw.GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "application_name")) != std::string::npos);

    std::wstring appData_unicode = fw.GetPlatform()->GetApplicationDataDirectoryW();
    BOOST_CHECK (appData_unicode.find(Core::ToWString(fw.GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "application_name"))) != std::wstring::npos);


    appData = fw.GetPlatform()->GetUserDocumentsDirectory();
    BOOST_CHECK (appData.find(fw.GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "application_name")) != std::string::npos);

    appData_unicode = fw.GetPlatform()->GetUserDocumentsDirectoryW();
    BOOST_CHECK (appData_unicode.find(Core::ToWString(fw.GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "application_name"))) != std::wstring::npos);
}

BOOST_AUTO_TEST_CASE( framework_application )
{
    Foundation::Framework fw;

    std::string app_name = Foundation::Application::Name();
    BOOST_CHECK_EQUAL (app_name, fw.GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "application_name") );

    std::wstring app_name_w = Foundation::Application::NameW();
    BOOST_CHECK (app_name_w.compare(Core::ToWString(fw.GetDefaultConfig().GetSetting<std::string>(Foundation::Framework::ConfigurationGroup(), "application_name"))) == 0 );
}

void frameworkConfigurationManagerTest()
{
    Foundation::ConfigurationManager manager(0,"./testing/configuration.xml");
    BOOST_CHECK_EQUAL (manager.DeclareSetting<std::string>("testGroup1", "test_key1", "default_value"), std::string("test_string"));
    BOOST_CHECK_EQUAL (manager.DeclareSetting<std::string>("testGroup1", "test_key1", std::string("default_value")), std::string("test_string"));
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

// Should be last, so default configuration file gets properly saved in debug mode
BOOST_AUTO_TEST_CASE( framework_test_module )
{
    Foundation::Framework fw;
    
    Test::StaticModuleDefinitions static_test;
    static_test(&fw);

    fw.GetModuleManager()->LoadAvailableModules();
    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Scene));
//    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Renderer));
    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Test));
//    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_WorldLogic));

    fw.Go();

    BOOST_CHECK( fw.GetComponentManager()->Begin(Test::EC_Dummy::NameStatic()) == fw.GetComponentManager()->End(Test::EC_Dummy::NameStatic()));
}


BOOST_AUTO_TEST_SUITE_END()
