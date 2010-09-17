/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   InputMapper.h
 *  @brief  Registers an InputContext from the Naali Input subsystem and uses it to translate
 *          given set of key and mouse sequences to Entity Actions on the entity the component is part of.
 */

#ifndef incl_EC_InputMapper_EC_InputMapper_h
#define incl_EC_InputMapper_EC_InputMapper_h

#include "ComponentInterface.h"
#include "Declare_EC.h"

#include <QMap>
#include <QKeySequence>
#include <QVector>
#include <QVariant>

/// Registers an InputContext from the Naali Input subsystem and uses it to translate
/// given set of key and mouse sequences to Entity Actions on the entity the component is part of.
class EC_InputMapper : public Foundation::ComponentInterface
{
    DECLARE_EC(EC_InputMapper);
    Q_OBJECT

public:
    /// Destructor.
    ~EC_InputMapper();

    /// ComponentInterface override. This component is serializable.
    virtual bool IsSerializable() const { return true; }

    /// This input mapper's input context priority.
    Attribute<QString> contextName;

    /// This input mapper's input context priority.
    Attribute<int> contextPriority;

    /// Does the mapper receive keyboard input events even when a Qt widget has focus.
    Attribute<bool> takeKeyboardEventsOverQt;

    /// Does the mapper receive mouse input events even when a Qt widget has focus.
    Attribute<bool> takeMouseEventsOverQt;

    /// Key sequence - action name mappings.
    Attribute<std::vector<QVariant> > mappings;

    typedef QMap<QKeySequence, QString> Mappings_t;

public slots:
    /** Register new key sequence - action mapping for this input mapper.
        @param keySeq Key sequence.
        @param action Name of the action. If you want to use parameters the string should look the following: 
        "More(Forward)" or "Move(Forward,100)" etc.
        @note If registering key sequence with modifier keys, don't use Qt::Key enum - use Qt::Modifer enum instead.
    */
    void RegisterMapping(const QKeySequence &keySeq, const QString &action);

    /// Returns the input context of this input mapper.
    InputContext *GetInputContext() const { return input_.get(); }

private:
    /** Constructor.
        @param module Declaring module.
    */
    explicit EC_InputMapper(IModule *module);

    /// Input context for this EC.
    boost::shared_ptr<InputContext> input_;

    /// List of registered key sequence - action mappings.
    Mappings_t mappings_;

private slots:
    /** Alters input context's parameters when attributes are changed.
        @param attribute Changed attribute.
        @param change Change type.
    */
    void AttributeUpdated(AttributeInterface *, AttributeChange::Type change);

    /** Handles key events from input service.
        Performs entity action for for the parent entity if action mapping is registered for the key event.
        @param e Key event.
    */
    void HandleKeyEvent(KeyEvent *e);

    /** Handles mouse events from input service.
        Performs entity action for for the parent entity if action mapping is registered for the mouse event.
        @param e Mouse event.
    */
    void HandleMouseEvent(MouseEvent *e);
};

#endif
