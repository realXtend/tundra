// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_Sound_EC_Sound_h
#define incl_EC_Sound_EC_Sound_h

#include "IComponent.h"
#include "IAttribute.h"
#include "ResourceInterface.h"
#include "Declare_EC.h"

namespace Foundation
{
    class ModuleInterface;
}

class EC_Sound : public IComponent
{
    DECLARE_EC(EC_Sound);
    Q_OBJECT
public:
    ~EC_Sound();
    virtual bool IsSerializable() const { return true; }

    Attribute<QString>      soundId_;
    Attribute<float>        soundInnerRadius_;
    Attribute<float>        soundOuterRadius_;
    Attribute<float>        soundGain_;
    Attribute<bool>         loopSound_;
    Attribute<bool>         triggerSound_;

public slots:
    void PlaySound();
    void StopSound();
    //! Get each attribute values and update sound's parameters if it's in playing state.
    void UpdateSoundSettings();

private slots:
    void UpdateSignals();
    void AttributeUpdated(IComponent *component, IAttribute *attribute);

private:
    explicit EC_Sound(IModule *module);
    ComponentPtr FindPlaceable() const;

    sound_id_t sound_id_;
};

#endif