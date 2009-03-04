// For conditions of distribution and use, see copyright notice in license.txt

#ifndef __incl_Interfaces_GuiServiceInterface_h__
#define __incl_Interfaces_GuiServiceInterface_h__

#include "ServiceInterface.h"

namespace Foundation
{
    class GuiServiceInterface : public ServiceInterface
    {
    public:
        GuiServiceInterface() {}
        virtual ~GuiServiceInterface() {}

        //! Creates new window widget
        virtual void createWindow() = 0;
        //! creates new widget
        virtual void createWidget() = 0;
        //! add new item to main menu
        virtual void addMenuItem() = 0;
    };
}

#endif // __incl_Interfaces_GuiSystemInterface_h__


