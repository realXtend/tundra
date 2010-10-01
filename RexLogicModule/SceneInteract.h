/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   SceneInteract.h
 *  @brief  Transforms generic mouse and keyboard input events to
*           input-related Entity Action for scene entities.
 */

#include "ForwardDefines.h"

#include <QObject>

#ifndef incl_RexLogicModule_SceneInteract_h
#define incl_RexLogicModule_SceneInteract_h

/// Transforms generic mouse and keyboard input events to input-related Entity Action for scene entities.
/** Peforms raycast each frame and if entity is hit, performs the following actions on it:
    -"MouseHoverIn"
    -"MouseHover"
    -"MouseHoverOut"
    -"MousePress"
*/
class SceneInteract : public QObject
{
    Q_OBJECT

public:
    /// Costructor.
    /** Framework takes ownership of this object
        @param fw Framework.
    */
    explicit SceneInteract(Foundation::Framework *fw);

    /// Destructor.
    ~SceneInteract() {}

signals:
    /// Emitted when scene was clicked and raycast hit an entity.
    /** @param entity Hit entity.
    */
    void EntityClicked(Scene::Entity *entity);

private:
    /// Peforms raycast to last known mouse cursor position.
    void Raycast();

    /// Framework.
    Foundation::Framework *framework_;

    /// Input context.
    InputContextPtr input_;

    /// Renderer pointer.
    Foundation::RendererWeakPtr renderer_;

    /// Last known mouse cursor's x position.
    int lastX_;

    /// Last known mouse cursor's y position.
    int lastY_;

    /// Last entity raycast has hit.
    Scene::EntityWeakPtr lastHitEntity_;

private slots:
    /// Executes "MouseHover" action each frame is raycast has hit and entity.
    void Update();

    /// Handles key events from input service.
    /** @param e Key event.
    */
    void HandleKeyEvent(KeyEvent *e);

    /// Handles mouse events from input service.
    /** @param e Mouse event.
    */
    void HandleMouseEvent(MouseEvent *e);
};

#endif
