// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Sound_EC_Sound_h
#define incl_EC_Sound_EC_Sound_h

#include "ComponentInterface.h"
#include "AttributeInterface.h"
#include "ResourceInterface.h"
#include "Declare_EC.h"

namespace Foundation
{
    class ModuleInterface;
}

class EC_Sound : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_Sound);
    Q_OBJECT
public:
    ~EC_Sound();
    virtual bool IsSerializable() const { return true; }

    Foundation::Attribute<QString>      soundId_;
    Foundation::Attribute<float>         soundInnerRadius_;
    Foundation::Attribute<float>         soundOuterRadius_;
    Foundation::Attribute<float>         soundGain_;
    Foundation::Attribute<bool>         loopSound_;
    Foundation::Attribute<bool>         triggerSound_;

public slots:
    void AttributeUpdated(Foundation::ComponentInterface *component, Foundation::AttributeInterface *attribute);
    void PlaySound();
    void StopSound();
    //! Get each attribute values and update sound's parameters if it's in playing state.
    void UpdateSoundSettings();

private slots:
    void UpdateSignals();

private:
    explicit EC_Sound(Foundation::ModuleInterface *module);
    Foundation::ComponentPtr FindPlaceable() const;

    sound_id_t sound_id_;
};

#endif