/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   G3dwhModule.h
 *  @brief  Brief description of the module goes here
 */

#ifndef G3DWHMODULE_H
#define G3DWHMODULE_H

#include "IModule.h"
#include "ModuleLoggingFunctions.h"

#include <QObject>
#include <QPointer>

#include "G3dwhDialog.h"
#include "Framework.h"


class G3dwhWidget;


namespace Foundation { class Framework; }


class G3dwhModule : public QObject, public IModule
{
    Q_OBJECT

public:
    /// Default constructor.
    G3dwhModule();

    /// Destructor.
    ~G3dwhModule();

    /// IModule override.
    void PreInitialize();

    /// IModule override.
    void Initialize();

    /// IModule override.
    void PostInitialize();

    /// IModule override.
    void Uninitialize();

    /// IModule override.
    void Update(f64 frametime);

    /// IModule override.
    bool HandleEvent(event_category_id_t category_id, event_id_t event_id, IEventData* data);


    MODULE_LOGGING_FUNCTIONS

    /// Returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return type_name_static_; }

private:
    /// Type name of the module.
    static std::string type_name_static_;

    /// Input context for this module.
    //boost::shared_ptr<InputContext> input_;

    //Foundation::Framework *framework_; //!< Parent framework.

    QPointer<G3dwhDialog> warehouse_;
    //Foundation::Framework *framework; ///< Framework.

private slots:
    void nullifyDialog();
public slots:
    void ShowG3dwhWindow();
};

#endif // G3dwhModule_H

