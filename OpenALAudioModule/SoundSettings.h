// For conditions of distribution and use, see copyright notice in license.txt

#include <QObject>

namespace Foundation
{
    class Framework;
}

class QWidget;

namespace OpenALAudio
{
    //! Sound settings editing window. Owned by OpenALAudioModule.
    class SoundSettings : public QObject
    {
        Q_OBJECT

    public:
        SoundSettings(Foundation::Framework* framework);
        ~SoundSettings();

    public slots:
        void MasterGainChanged(int value);
        void TriggeredGainChanged(int value);
        void AmbientGainChanged(int value);
        void VoiceGainChanged(int value);

    private:
        Foundation::Framework* framework_;
        QWidget *settings_widget_;
    };
}
