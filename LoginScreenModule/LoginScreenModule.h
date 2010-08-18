/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   
 *  @brief  
 */

#ifndef incl_LoginScreenModule_LoginScreenModule_h
#define incl_LoginScreenModule_LoginScreenModule_h

#include "ModuleInterface.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>

class KeyEvent;
class InputContext;

class LoginWidget;

class LoginScreenModule : public QObject, public Foundation::ModuleInterface
{
    Q_OBJECT

public:
    /// Default constructor.
    LoginScreenModule();

    /// Destructor.
    ~LoginScreenModule();

    /// ModuleInterface override.
    void PreInitialize();

    /// ModuleInterface override.
    void Initialize();

    /// ModuleInterface override.
    void PostInitialize();

    /// ModuleInterface override.
    void Uninitialize();

    /// ModuleInterface override.
    void Update(f64 frametime);

    /// ModuleInterface override.
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, Foundation::EventDataInterface* data);

    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

private:
    /// Type name of the module.
    static std::string type_name_static_;

    /// Login window.
    class LoginWidget *window_;

    /// Login window.
    boost::shared_ptr<InputContext> input_;

private slots:
    /// Handles key event.
    void HandleKeyEvent(KeyEvent *key);
};

#endif
