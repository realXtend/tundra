// For conditions of distribution and use, see copyright notice in license.txt

#include <QObject>
#include "InputServiceInterface.h"
namespace Foundation
{
    class Framework;
}
class KeyEvent;
namespace OgreRenderer
{
    //! Widget for rendering settings
    class RendererSettings : public QObject
    {
        Q_OBJECT

    public:
        RendererSettings(Foundation::Framework* framework);
        ~RendererSettings();

    public slots:
        void ViewDistanceChanged(double value);
        void ShadowQualityChanged(int quality);
        
        //! toggles fullscreen mode
        void SetFullScreenMode(bool value);

        void KeyPressed(KeyEvent* e);


    private:
        void InitWindow();
        InputContextPtr input_context_;

        Foundation::Framework* framework_;

        //! Widget for renderer settings
        QWidget *settings_widget_;

    };
}