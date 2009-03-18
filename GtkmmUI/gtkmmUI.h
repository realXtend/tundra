#ifndef incl_GtkmmUI_h
#define incl_GtkmmUI_h

#include "ModuleInterface.h"

class GtkmmUIImpl;

/// GtkmmUI is a module that implements and provides Gtkmm UI for other modules.
class GtkmmUI : public Foundation::ModuleInterface_Impl
{
public:
    GtkmmUI();
    virtual ~GtkmmUI();

    void Load();
    void Unload();
    void Initialize(Foundation::Framework *framework);
    void Uninitialize(Foundation::Framework *framework);

    void Update();

private:
    void operator=(const GtkmmUI &);
    GtkmmUI(const GtkmmUI &);

    Foundation::Framework *framework_;

    boost::shared_ptr<GtkmmUIImpl> impl_;
};

#endif
