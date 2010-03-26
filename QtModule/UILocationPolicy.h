// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UILocationPolicy_h
#define incl_QtModule_UILocationPolicy_h

#include "QtModuleApi.h"

namespace QtUI
{
    class QT_MODULE_API UILocationPolicy
    {   
    public:
        UILocationPolicy() : stationary_(false), always_on_top_(false) {}
        virtual ~UILocationPolicy() {}
        
        virtual void SetAlwaysOnTop(bool on) { always_on_top_ = on; }
        virtual bool IsAlwaysOnTop() const { return always_on_top_; }

        virtual void SetStationary(bool on) { stationary_ = on; }
        virtual bool IsStationary() const { return stationary_; }
       

    private:
        bool stationary_;
        bool always_on_top_;
    };
}

#endif