// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_GuiServiceInterface_h
#define incl_Interfaces_GuiServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    class GuiServiceInterface : public ServiceInterface
    {
    public:
        GuiServiceInterface() {}
        virtual ~GuiServiceInterface() {}

        //! Creates new window widget
        virtual void CreateGuiWindow() = 0;
        //! creates new widget
        virtual void CreateWidget() = 0;
        //! add new item to main menu
        virtual void AddMenuItem() = 0;
    };
}

#endif // incl_Interfaces_GuiSystemInterface_h


