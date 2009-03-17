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
    Foundation::Console::CommandResult TestCallbackSuccess(const Core::StringVector &params)
    {
        BOOST_CHECK_EQUAL(params[0], "paramA");
        BOOST_CHECK_EQUAL(params[1], "paramB");

        Foundation::Console::CommandResult result = {true, "Success"};
        return result;
    }
};

BOOST_AUTO_TEST_CASE( support_modules_console )
{
    Foundation::Framework fw;
    
    fw.GetModuleManager()->LoadModuleByName("SupportModules", "Console");
    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Console));
    
    Foundation::Console::ConsoleServiceInterface *console = fw.GetService<Foundation::Console::ConsoleServiceInterface>
        (Foundation::Service::ST_Console);

    TestA test_class;
    Foundation::Console::Command command = {"Test_Command", "Test command", Foundation::Console::Bind(&test_class, TestA::TestCallbackSuccess) };
    console->RegisterCommand(command);

    console->ExecuteCommand("test_command (paramA, paramB )");

    

    fw.GetModuleManager()->UninitializeModules();
    fw.GetModuleManager()->UnloadModules();
}


BOOST_AUTO_TEST_SUITE_END()

