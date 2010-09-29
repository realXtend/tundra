#ifndef incl_Core_Ui_h
#define incl_Core_Ui_h

#include <QObject>
#include <memory>

#include "NaaliUiFwd.h"
#include "UiApi.h"

class UI_API NaaliUi : public QObject
{
    Q_OBJECT;

public:
    explicit NaaliUi(Foundation::Framework *owner);
    NaaliUi::~NaaliUi();

public slots:
    NaaliMainWindow *MainWindow() const;

    NaaliGraphicsView *GraphicsView() const;

    QGraphicsScene *GraphicsScene() const;

private:
    Foundation::Framework *owner;

    NaaliMainWindow *mainWindow;
    NaaliGraphicsView *graphicsView;
    QGraphicsScene *graphicsScene;
};

#endif
