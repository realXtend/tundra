// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderingModule_StereoWidget_h
#define incl_OgreRenderingModule_StereoWidget_h

#include <QWidget>
#include "ui_stereoUI.h"
namespace Foundation
{
    class Framework;
}
namespace OgreRenderer
{
    
	class StereoWidget: public QWidget, private Ui::StereoUI 
    {
        Q_OBJECT
    public:
        StereoWidget( Foundation::Framework* framework, QWidget* parent=0 );


    private:
        Foundation::Framework* framework_;

    };

}
#endif