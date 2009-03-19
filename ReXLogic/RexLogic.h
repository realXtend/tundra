// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_System_h
#define incl_RexLogic_System_h

#include "ModuleInterface.h"
#include "NetworkEventHandler.h"

//! interface for modules
class RexLogic : public Foundation::ModuleInterface_Impl
{
public:
    RexLogic();
    virtual ~RexLogic();

    virtual void Load();
    virtual void Unload();
    virtual void Initialize(Foundation::Framework *framework);
    virtual void PostInitialize(Foundation::Framework *framework);
    virtual void Uninitialize(Foundation::Framework *framework);

    virtual void Update();
    
    virtual bool HandleEvent(Core::event_category_id_t category_id, Core::event_id_t event_id, Foundation::EventDataInterface* data);
    
    MODULE_LOGGING_FUNCTIONS;

    //! returns name of this module. Needed for logging.
    static const std::string &NameStatic() { return Foundation::Module::NameFromType(type_static_); }

    static const Foundation::Module::Type type_static_ = Foundation::Module::MT_WorldLogic;

private:
    Foundation::Framework *framework_;
    
    NetworkEventHandler *network_handler_; 

    typedef boost::function<bool(Core::event_id_t,Foundation::EventDataInterface*)> LogicEventHandlerFunction;
    typedef std::map<Core::event_category_id_t, LogicEventHandlerFunction> LogicEventHandlerMap;
    LogicEventHandlerMap event_handlers_;
};

#endif
