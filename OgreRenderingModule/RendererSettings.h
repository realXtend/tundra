// For conditions of distribution and use, see copyright notice in license.txt

#include <QObject>

namespace Foundation
{
    class Framework;
}

namespace OgreRenderer
{
    //! Sound settings editing window. Owned by OpenALAudioModule.
    class RendererSettings : public QObject
    {
        Q_OBJECT

    public:
        RendererSettings(Foundation::Framework* framework);
        ~RendererSettings();

    public slots:
        void ViewDistanceChanged(double value);

    private:
        void InitWindow();

        Foundation::Framework* framework_;

        //! Widget for renderer settings
        QWidget *settings_widget_;

    };
}