// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"
#include "ServiceInterfaces.h"

//! Unit test for framework
BOOST_AUTO_TEST_SUITE(test_suite_support_modules)


struct TestA
{
    Console::CommandResult TestCallbackSuccess(const Core::StringVector &params)
    {
        BOOST_CHECK_EQUAL(params[0], "paramA");
        BOOST_CHECK_EQUAL(params[1], "paramB");

        return Console::ResultSuccess("Success");
    }
    Console::CommandResult TestCallbackFailure(const Core::StringVector &params)
    {
        BOOST_CHECK_EQUAL(params.size(), 0);

        return Console::ResultFailure();
    }
};

struct TestB
{
    Console::CommandResult TestCallbackThreaded(const Core::StringVector &params)
    {
        test_var_ = 1;
        return Console::ResultSuccess();
    }

    void operator()()
    {
        test_var_ = 0;
        Console::CommandService *console = fw_->GetService<Console::CommandService>
            (Foundation::Service::ST_ConsoleCommand);

        console->RegisterCommand("Test CommandC", "Test command threaded", Console::Bind(this, &TestB::TestCallbackThreaded), true);

        console->Update();
        boost::optional<Console::CommandResult> result;
        while ( !(result = console->Poll("Test CommandC")) )
            console->Update();
        
        BOOST_CHECK_EQUAL (result->success_, true);
    }
    Foundation::Framework *fw_;
    int test_var_;
};

BOOST_AUTO_TEST_CASE( support_modules_console )
{
    Foundation::Framework fw;
    
    fw.GetModuleManager()->LoadModuleByName("SupportModules", "ConsoleModule");
    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Console));
    
    Console::CommandService *console = fw.GetService<Console::CommandService>
        (Foundation::Service::ST_ConsoleCommand);

    TestA test_class;
    console->RegisterCommand("Test_CommandA", "Test command Success", Console::Bind(&test_class, &TestA::TestCallbackSuccess));
    console->RegisterCommand("Test_CommandB", "Test command Failure", Console::Bind(&test_class, &TestA::TestCallbackFailure));
    

    Console::CommandResult result = console->ExecuteCommand("Test_CommandA (paramA, paramB )");
    BOOST_CHECK_EQUAL (result.success_, true);
    BOOST_CHECK_EQUAL (result.why_, "Success");

    result = console->ExecuteCommand("Test_CommandB");
    BOOST_CHECK_EQUAL (result.success_, false);
    BOOST_CHECK_EQUAL (result.why_.size(), 0);

    TestB testb;
    testb.fw_ = &fw;

    Core::Thread thread(boost::ref(testb));
    console->QueueCommand("Test CommandC");
    thread.join();
    
    BOOST_CHECK_EQUAL (testb.test_var_, 1);

    fw.GetModuleManager()->UninitializeModules();
    fw.GetModuleManager()->UnloadModules();
}


BOOST_AUTO_TEST_SUITE_END()

