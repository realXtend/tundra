/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   AddContentWindow.h
 *  @brief  Window for adding new content and assets.
 */

#ifndef incl_SceneStructureModule_AddContentWindow_h
#define incl_SceneStructureModule_AddContentWindow_h

#include <QWidget>

#include "ForwardDefines.h"
#include "SceneDesc.h"
#include "Vector3D.h"

class QTreeWidget;
class QPushButton;
class QComboBox;

/// Window for adding new content and assets.
/** The window is modal and is deleted when it's closed.
*/
class AddContentWindow : public QWidget
{
    Q_OBJECT

public:
    /// Constructs the window.
    /** @param dest Destination scene.
        @param parent Parent widget.
    */
    explicit AddContentWindow(const Scene::ScenePtr &dest, QWidget *parent = 0);

    ///
    ~AddContentWindow();

    /// Add scene descrition to be shown the tree widgets.
    /** @param desc Scene description.
    */
    void AddDescription(const SceneDesc &desc);

    /// 
    /** @param pos
    */
    void AddPosition(const Vector3df &pos) { position = pos; }

private:
    Q_DISABLE_COPY(AddContentWindow)
    QTreeWidget *entityTreeWidget; ///< Tree widget showing entities.
    QTreeWidget *assetTreeWidget; ///< Tree widget showing asset references.
    Scene::SceneWeakPtr scene; ///< Destination scene.
    SceneDesc sceneDesc; ///< Current scene description shown on the window.
    QPushButton *addContentButton; ///< Add content button.
    QPushButton *cancelButton; ///< Cancel/close button.
    QComboBox *storageComboBox; ///< Asset storage combo box.
    Vector3df position; ///< Centralization position for instantiated context (if used).

private slots:
    /// Checks all entity check boxes.
    void SelectAllEntities();

    /// Unchecks all entity check boxes.
    void DeselectAllEntities();

    /// Checks all asset check boxes.
    void SelectAllAssets();

    /// Unchecks all asset check boxes.
    void DeselectAllAssets();

    /// Start content creation and asset uploading.
    void AddContent();

    /// Closes the window.
    void Close();
};

#endif
