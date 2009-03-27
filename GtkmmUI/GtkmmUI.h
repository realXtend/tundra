#ifndef incl_gtkmmUI_h
#define incl_gtkmmUI_h

#include "Foundation.h"
#include "ModuleInterface.h"

class GtkmmUIImpl;

/// GtkmmUI is a module that implements and provides Gtkmm UI for other modules.
class GtkmmUI : public Foundation::ModuleInterfaceImpl
{
public:
    GtkmmUI();
    virtual ~GtkmmUI();

    void Load();
    void Unload();
    void Initialize();
    void Uninitialize();
    void Update();

private:
    void operator=(const GtkmmUI &);
    GtkmmUI(const GtkmmUI &);

    Foundation::Framework *framework_;

    boost::shared_ptr<GtkmmUIImpl> impl_;
};

#endif
