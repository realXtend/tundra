// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Core_Ui_h
#define incl_Core_Ui_h

#include <QObject>
#include <memory>

#include "NaaliUiFwd.h"
#include "UiApi.h"

/// NaaliUi is the core API object exposed for all UI-related functionality.
/// \todo Currently the existing UiServiceInterface is still intact, and that should be used to access the functionality for
/// creating new widgets to the scene. See Framework::UiService. The UiService will at some point be removed and migrated to
/// this.
class UI_API NaaliUi : public QObject
{
    Q_OBJECT;

public:
    explicit NaaliUi(Foundation::Framework *owner);
    ~NaaliUi();

public slots:
    /// Returns the widget that is the main window of Naali.
    NaaliMainWindow *MainWindow() const;

    /// This widget shows all the user-added UI that has been added to the screen. It is contained as a child of NaaliMainWindow.
    NaaliGraphicsView *GraphicsView() const;

    /// Returns the Naali main QGraphicsScene, which hosts all the user-added widgets.
    QGraphicsScene *GraphicsScene() const;

private:
    Foundation::Framework *owner;

    NaaliMainWindow *mainWindow;
    NaaliGraphicsView *graphicsView;
    QGraphicsScene *graphicsScene;
};

#endif
