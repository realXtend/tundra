// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"
#include "ServiceInterface.h"
#include "StaticModuleDefinitions.h"
#include "EC_Dummy.h"
#include "TestModuleb.h"
#include "ConfigurationManager.h"
#include "CoreStringUtils.h"
#include "Application.h"

//! Unit test for framework
BOOST_AUTO_TEST_SUITE(fw)

BOOST_AUTO_TEST_CASE( platform )
{
    Foundation::Framework fw(0, NULL);

    std::string appData = fw.GetPlatform()->GetApplicationDataDirectory();
    BOOST_CHECK (appData.find(fw.GetDefaultConfig().GetSetting<std::string>(
        Foundation::Framework::ConfigurationGroup(), "application_name")) != std::string::npos);

    std::wstring appData_unicode = fw.GetPlatform()->GetApplicationDataDirectoryW();
    BOOST_CHECK (appData_unicode.find(ToWString(fw.GetDefaultConfig().GetSetting<std::string>(
        Foundation::Framework::ConfigurationGroup(), "application_name"))) != std::wstring::npos);


    appData = fw.GetPlatform()->GetUserDocumentsDirectory();
    BOOST_CHECK (appData.find(fw.GetDefaultConfig().GetSetting<std::string>(
        Foundation::Framework::ConfigurationGroup(), "application_name")) != std::string::npos);

    appData_unicode = fw.GetPlatform()->GetUserDocumentsDirectoryW();
    BOOST_CHECK (appData_unicode.find(ToWString(fw.GetDefaultConfig().GetSetting<std::string>(
        Foundation::Framework::ConfigurationGroup(), "application_name"))) != std::wstring::npos);
}

BOOST_AUTO_TEST_CASE( application )
{
    Foundation::Framework fw(0, NULL);

    std::string app_name = Foundation::Application::Name();
    BOOST_CHECK_EQUAL (app_name, fw.GetDefaultConfig().GetSetting<std::string>(
        Foundation::Framework::ConfigurationGroup(), "application_name") );

    std::wstring app_name_w = Foundation::Application::NameW();
    BOOST_CHECK (app_name_w.compare(ToWString(fw.GetDefaultConfig().GetSetting<std::string>(
        Foundation::Framework::ConfigurationGroup(), "application_name"))) == 0 );
}


#ifdef PROFILING
int total_recursions = 5;
void Profiler_Recursion()
{
    PROFILE(Test_Profile5);
    static int cnt = 0;
    cnt++;
    if (cnt < total_recursions)
        Profiler_Recursion();
}


BOOST_AUTO_TEST_CASE( profiler )
{
    Foundation::Framework fw(0, NULL);
    {
        
        int u;
        PROFILE(Test_Profile1);
        {
            for (int k = 0 ; k < 2 ; ++k)
            {
                PROFILE(Test_Profile2);
                for (int i = 0 ; i<4 ; ++i)
                {
                    PROFILE(Test_Profile4);
                    u = 5 * i / 2;
                }
            }
            PROFILE(Test_Profile3);
        }
        ELIFORP(Test_Profile1);

        // recursion test
        Profiler_Recursion();
        

        Foundation::Profiler &profiler = fw.GetProfiler();
        Foundation::ProfilerNodeTree *all_root = profiler.GetRoot();
        Foundation::ProfilerNodeTree *root = all_root->GetChild(profiler.GetThisThreadRootBlockName());

        Foundation::ProfilerNode *node = static_cast<Foundation::ProfilerNode*>(root->GetChild("Test_Profile1"));
        BOOST_CHECK (node != NULL);
        BOOST_CHECK_EQUAL (node->num_called_total_, 1);
        
        node = static_cast<Foundation::ProfilerNode*>(node->GetChild("Test_Profile3"));
        BOOST_CHECK (node != NULL);
        node = static_cast<Foundation::ProfilerNode*>(node->Parent()->GetChild("Test_Profile2"));
        BOOST_CHECK (node != NULL);
        BOOST_CHECK_EQUAL (node->num_called_total_, 2);
        node = static_cast<Foundation::ProfilerNode*>(node->GetChild("Test_Profile4"));
        BOOST_CHECK (node != NULL);
        BOOST_CHECK_EQUAL (node->num_called_total_, 8);

        node = static_cast<Foundation::ProfilerNode*>(root->GetChild("Test_Profile5"));
        BOOST_CHECK (node != NULL);
        BOOST_CHECK_EQUAL (node->num_called_total_, total_recursions);
    }
}
#endif

void frameworkConfigurationManagerTest()
{
    Foundation::ConfigurationManager manager(0, "./testing/");
    BOOST_CHECK_EQUAL (manager.DeclareSetting<std::string>("Configuration", "test_key1", "default_value"), std::string("test_string"));
    BOOST_CHECK_EQUAL (manager.DeclareSetting<std::string>("Configuration", "test_key1", std::string("default_value")), std::string("test_string"));
    BOOST_CHECK_EQUAL (manager.DeclareSetting("TestGroup2", "test_key2", 0), 123456);
    BOOST_CHECK_EQUAL (manager.DeclareSetting("TestGroup2", "test_key3", false), true);
    BOOST_CHECK (equals(manager.DeclareSetting("TestGroup2", "test_key4", 0.f), 3.2f));
    BOOST_CHECK (manager.HasKey("Configuration", "test_key1"));
    BOOST_CHECK (manager.HasKey("NoGroup", "no_key") == false);
}

BOOST_AUTO_TEST_CASE( configuration_manager )
{
    frameworkConfigurationManagerTest();

#ifdef _DEBUG
     // twice, to check config file gets properly written in debug mode
    frameworkConfigurationManagerTest();
#endif
}

BOOST_AUTO_TEST_CASE( headless )
{
    // headless mode not work with qt currently

    //// integration test for running the viewer in headless mode
    //Foundation::Framework fw;
    //
    //Test::StaticModuleDefinitions static_test;
    //static_test(&fw);

    //fw.GetModuleManager()->ExcludeModule(Foundation::Module::MT_Test);
    //fw.GetModuleManager()->ExcludeModule(Test::TestModuleB::NameStatic());
    //fw.GetModuleManager()->ExcludeModule("GtkmmUI");
    //fw.GetModuleManager()->ExcludeModule(Foundation::Module::MT_Renderer);
    //fw.GetModuleManager()->ExcludeModule("CommunicationUIModule");
    //fw.GetModuleManager()->ExcludeModule(Foundation::Module::MT_CommunicationUI);
    //
    //fw.PostInitialize();
    //fw.GetModuleManager()->UpdateModules(0.01);
    //Console::CommandManagerPtr command = fw.GetService<Console::CommandService>(Foundation::Service::ST_ConsoleCommand).lock();
    //command->QueueCommand("login");
    //boost::timer timer;
    //Foundation::RootLogInfo("Now running headless mode integration test. Please wait ~10 seconds.");
    //while (timer.elapsed() < 15.0) // pretend 10 seconds is enought time to login
    //{
    //    fw.GetModuleManager()->UpdateModules(0.01);
    //    fw.GetEventManager()->ProcessDelayedEvents(0.01);
    //}

    //command->QueueCommand("sendevent(Input, 4)");
    //fw.GetModuleManager()->UpdateModules(0.01);
    //command->QueueCommand("sendevent(Input, 5)");
    //fw.GetModuleManager()->UpdateModules(0.01);

    //timer.restart();
    //command->QueueCommand("logout");
    //while (timer.elapsed() < 7.0)
    //    fw.GetModuleManager()->UpdateModules(0.01);

    //fw.UnloadModules();
}

// Should be last, so default configuration file gets properly saved in debug mode
BOOST_AUTO_TEST_CASE( test_module )
{
    Foundation::Framework fw(0, NULL);

    Test::StaticModuleDefinitions static_test;
    static_test(&fw);

    fw.GetModuleManager()->LoadAvailableModules();
//    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Renderer));
    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Test));
//    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_WorldLogic));

    fw.Go();

///\todo ComponentManager::Begin() and End() doesn't exist anymore
//    BOOST_CHECK(fw.GetComponentManager()->Begin(Test::EC_Dummy::NameStatic()) == fw.GetComponentManager()->End(Test::EC_Dummy::NameStatic()));
}


BOOST_AUTO_TEST_SUITE_END()
