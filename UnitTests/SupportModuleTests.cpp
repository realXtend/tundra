// For conditions of distribution and use, see copyright notice in license.txt

#include <boost/test/unit_test.hpp>

//#include <OISKeyboard.h>

#include "CoreStdIncludes.h"
#include "Core.h"
#include "Foundation.h"
#include "ServiceInterface.h"
#include "StaticModuleDefinitions.h"

#include "ConsoleModuleApi.h"
#include "ConsoleModule.h"
#include "ConsoleManager.h"
#include "OgreOverlay.h"

#include "InputEvents.h"

///\todo Depends on OIS and OgreConsole which both are deprecated.
/*
//! Unit test for framework
BOOST_AUTO_TEST_SUITE(support_modules)

Foundation::Framework &CreateFramework()
{
    static Foundation::Framework fw(0, NULL);
    return fw;
}

struct TestA
{
    Console::CommandResult TestCallbackSuccess(const StringVector &params)
    {
        BOOST_CHECK_EQUAL(params[0], "paramA");
        BOOST_CHECK_EQUAL(params[1], "paramB");

        return Console::ResultSuccess("Success");
    }
    Console::CommandResult TestCallbackFailure(const StringVector &params)
    {
        BOOST_CHECK_EQUAL(params.size(), 0);

        return Console::ResultFailure();
    }
};

struct TestB
{
    Console::CommandResult TestCallbackThreaded(const StringVector &params)
    {
        test_var_ = 1;
        return Console::ResultSuccess();
    }

    void operator()()
    {
        test_var_ = 0;
        boost::shared_ptr<Console::CommandService> console = fw_->GetService<Console::CommandService>
            (Foundation::Service::ST_ConsoleCommand).lock();

        console->RegisterCommand(Console::CreateCommand("Test CommandC", "Test command threaded", Console::Bind(this, &TestB::TestCallbackThreaded), true));

        console->Update();
        boost::optional<Console::CommandResult> result;
        while ( !(result = console->Poll("Test CommandC")) )
            console->Update();
        
        BOOST_CHECK_EQUAL (result->success_, true);
    }
    Foundation::Framework *fw_;
    int test_var_;
};

BOOST_AUTO_TEST_CASE( console_commands )
{
    Foundation::Framework &fw = CreateFramework();


    fw.GetModuleManager()->ExcludeModule("StaticModuleTest");

    //! \todo Gtk seems to cause problems with unit tests, so excluded for now
    //! \bug Re-creating Gtk::Main when running unit tests in debug mode causes a crash. 
    //!      It should have been destroyed properly though, and unloading/loading GtkmmUI
    //!      module at runtime works just fine. The problem seems to be confined to
    //!      unit tests in debug mode (run outside of the debugger).
    fw.GetModuleManager()->ExcludeModule("GtkmmUI");

    Test::StaticModuleDefinitions static_test;
    static_test(&fw);

    fw.GetModuleManager()->LoadModuleByName("OgreRenderingModule", "OgreRenderingModule");
    fw.GetModuleManager()->LoadModuleByName("SupportModules", "ConsoleModule");
    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Console));
    BOOST_CHECK (fw.GetModuleManager()->HasModule(Foundation::Module::MT_Renderer));
    
    boost::shared_ptr<Console::CommandService> console = fw.GetService<Console::CommandService>
        (Foundation::Service::ST_ConsoleCommand).lock();

    TestA test_class;
    console->RegisterCommand(Console::CreateCommand("Test_CommandA", "Test command Success", Console::Bind(&test_class, &TestA::TestCallbackSuccess)));
    console->RegisterCommand(Console::CreateCommand("Test_CommandB", "Test command Failure", Console::Bind(&test_class, &TestA::TestCallbackFailure)));
    

    Console::CommandResult result = console->ExecuteCommand("Test_CommandA (paramA, paramB )");
    BOOST_CHECK_EQUAL (result.success_, true);
    BOOST_CHECK_EQUAL (result.why_, "Success");

    result = console->ExecuteCommand("Test_CommandB");
    BOOST_CHECK_EQUAL (result.success_, false);
    BOOST_CHECK_EQUAL (result.why_.size(), 0);

    TestB testb;
    testb.fw_ = &fw;

    Thread thread(boost::ref(testb));
    console->QueueCommand("Test CommandC");
    thread.join();
    
    BOOST_CHECK_EQUAL (testb.test_var_, 1);
}

BOOST_AUTO_TEST_CASE( console_ogre )
{
    Foundation::Framework &fw = CreateFramework();

    Console::ConsoleManager *console_manager = checked_static_cast<Console::ConsoleManager*>(fw.GetService<Console::ConsoleServiceInterface>
        (Foundation::Service::ST_Console).lock().get());

    Console::ConsolePtr ogre_console = console_manager->GetOgre();
    Console::OgreOverlay *overlay = checked_static_cast<Console::OgreOverlay*>(ogre_console.get());
    
    overlay->SetVisible(true);
    BOOST_CHECK_EQUAL(overlay->IsVisible(), true);
    overlay->Update(60); // fast-forward one minute
    BOOST_CHECK_EQUAL(overlay->IsVisible(), true);
    BOOST_CHECK_EQUAL(overlay->IsActive(), true);

    
    overlay->Scroll(30000);
    overlay->Update(60);
    overlay->Scroll(-30000);
    overlay->Update(60);
    overlay->Scroll(300000);
    overlay->Update(60);
    overlay->Scroll(-2);
    overlay->Update(60);
    overlay->Scroll(1);
    overlay->Update(60);
    overlay->Clear();
    overlay->Update(60);
    overlay->Scroll(5);
    overlay->Update(60);
    overlay->Scroll(-5);
    overlay->Update(60);
    overlay->Print("Test message A");
    overlay->Print("Test message B");
    overlay->Print("Test message C");
    overlay->Update(60);
    overlay->Scroll(1);
    overlay->Update(60);
    overlay->Scroll(6);
    overlay->Update(60);
    overlay->Scroll(-6);
    overlay->Update(60);
    BOOST_CHECK_EQUAL(overlay->GetLine(0), std::string("Test message C"));
    BOOST_CHECK_EQUAL(overlay->GetLine(1), std::string("Test message B"));
    BOOST_CHECK_EQUAL(overlay->GetLine(2), std::string("Test message A"));

    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string(""));

    overlay->HandleKeyDown(OIS::KC_A, 65);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string("A"));
    overlay->HandleKeyDown(OIS::KC_B, 66);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string("AB"));
    overlay->HandleKeyDown(OIS::KC_BACK, 0);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string("A"));
    overlay->HandleKeyDown(OIS::KC_C, 67);
    overlay->HandleKeyDown(OIS::KC_RIGHT, 0);
    overlay->HandleKeyDown(OIS::KC_LEFT, 0);
    overlay->HandleKeyDown(OIS::KC_BACK, 0);
    overlay->HandleKeyDown(OIS::KC_DELETE, 0);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string(""));

    overlay->HandleKeyDown(OIS::KC_A, 65);
    overlay->HandleKeyDown(OIS::KC_RETURN, 0);
    overlay->HandleKeyDown(OIS::KC_B, 66);
    overlay->HandleKeyDown(OIS::KC_RETURN, 0);
    overlay->HandleKeyDown(OIS::KC_C, 67);
    overlay->HandleKeyDown(OIS::KC_RETURN, 0);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string(""));

    overlay->HandleKeyDown(OIS::KC_DOWN, 0);
    overlay->HandleKeyDown(OIS::KC_UP, 0);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string("C"));
    overlay->HandleKeyDown(OIS::KC_UP, 0);
    overlay->HandleKeyDown(OIS::KC_UP, 0);
    overlay->HandleKeyDown(OIS::KC_UP, 0);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string("A"));
    overlay->HandleKeyDown(OIS::KC_DOWN, 0);
    BOOST_CHECK_EQUAL(overlay->GetCommandLine(), std::string("B"));


    overlay->SetVisible(false);
    BOOST_CHECK_EQUAL(overlay->IsActive(), false);


    ogre_console.reset();
    fw.GetModuleManager()->UninitializeModules();
    fw.GetModuleManager()->UnloadModules();
}


BOOST_AUTO_TEST_SUITE_END()
*/

