// For conditions of distribution and use, see copyright notice in license.txt

#include <QObject>

namespace Foundation
{
    class Framework;
}

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

        //! toggles fullscreen mode
        void SetFullScreenMode(bool value);

    private:
        void InitWindow();

        Foundation::Framework* framework_;

        //! Widget for renderer settings
        QWidget *settings_widget_;

    };
}