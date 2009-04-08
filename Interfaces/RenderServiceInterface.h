// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_RenderServiceInterface_h
#define incl_Interfaces_RenderServiceInterface_h

#include "ServiceInterface.h"

namespace Foundation
{
    class RenderServiceInterface : public ServiceInterface
    {
    public:
        RenderServiceInterface() {}
        virtual ~RenderServiceInterface() {}

        virtual void Render() = 0;
        virtual void Raycast() = 0;
        //! Returns main window handle, or 0 if no window is opened
        virtual size_t GetWindowHandle() const = 0;
    };
}

#endif

