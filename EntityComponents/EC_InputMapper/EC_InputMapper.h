/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InputMapper.h
 *  @brief  Registers an InputContext from the Naali Input subsystem and uses it to translate
 *          given set of keys to Entity Actions on the entity the component is part of.
 */

#ifndef incl_EC_InputMapper_EC_InputMapper_h
#define incl_EC_InputMapper_EC_InputMapper_h

#include "ComponentInterface.h"
#include "Declare_EC.h"

#include <QMap>
#include <QKeySequence>

class KeyEvent;
class InputContext;

/** Registers an InputContext from the Naali Input subsystem and uses it to translate
 *  given set of keys to Entity Actions on the entity the component is part of.
 */
class EC_InputMapper : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_InputMapper);
    Q_OBJECT

public:
    /// Destructor.
    ~EC_InputMapper();

    typedef QMap<QKeySequence, QString> Mappings_t;

public slots:
    /** Register new key sequence - action mapping for this input mapper.
     *  @param action Name of the action.
     *  @param keySeq Key sequence.
     */
    void RegisterMapping(const QString &action, const QKeySequence &keySeq);

private:
    /** Constructor.
     *  @param module Declaring module.
     */
    explicit EC_InputMapper(Foundation::ModuleInterface *module);

    /// Input context for this EC.
    boost::shared_ptr<InputContext> input_;

    /// List of registered key sequence - action mappings.
    Mappings_t mappings_;

private slots:
    /** Handles key events from input service.
     *  Performs entity action for for the parent entity if action mapping is registered for the key event.
     *  @param key Key event.
     */
    void HandleKeyEvent(KeyEvent *key);
};

#endif
