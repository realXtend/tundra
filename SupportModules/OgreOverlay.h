// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ConsoleOgre_h
#define incl_ConsoleOgre_h

#include "ModuleInterface.h"
//#include "EC_OgreConsoleOverlay.h"

namespace Foundation
{
    class Framework;
}

namespace Console
{
    class ConsoleModule;

    //! Ogre debug console
    class OgreOverlay : public Console::ConsoleServiceInterface, boost::noncopyable
    {
        OgreOverlay();
    public:
        //! constructor that takes parent module
        OgreOverlay(Foundation::ModuleInterface *module);

        //! destructor
        virtual ~OgreOverlay();

        virtual void Print(const std::string &text);

    private:
        //! entity component
        Foundation::ComponentPtr console_overlay_;
        
        //! parent module
        Foundation::ModuleInterface *module_;
    };
}

#endif
