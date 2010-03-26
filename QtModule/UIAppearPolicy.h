// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_UIAppearPolicy_h
#define incl_QtModule_UIAppearPolicy_h

#include "QtModuleApi.h"

namespace QtUI
{
    class QT_MODULE_API UIAppearPolicy
    {
    public:
        UIAppearPolicy() : resizable_(true) {}
        virtual ~UIAppearPolicy() {}
    
         /**
         * Enables or disables alpha fading animation. 
         * @param on is alpha fade on or off. 
         * @param time is how fast canvas is "animated" default value 2 s
         */
        
         //void SetAlphaFade(bool on, double time = 2) { total_dur_ = time, use_fading_ = on; }

        virtual void SetResizable(bool enable) { resizable_ = enable; } 
        virtual bool IsResizable() const { return resizable_; }
        
    private:

        /// If true, this canvas cannot be moved by dragging the mouse on the window title. Only functional if
        /// mode=Internal.
        //bool locked_;
      
        //bool resize_locked_;

        //bool always_top_;
       
        // For alpha fade feature:

        /*
        bool fade_;
        double alpha_;
        double current_dur_;
      
        double total_dur_;
        QTime clock_;
        int lastTime_;

        bool fade_on_hiding_;
        bool use_fading_;
        */

        bool resizable_;



    };

}

#endif 