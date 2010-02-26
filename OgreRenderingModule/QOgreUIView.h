// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_QOgreUIView_h
#define incl_OgreRenderer_QOgreUIView_h

//#include "StableHeaders.h"

#include "QOgreWorldView.h"

#include <OgreRenderWindow.h>

#include <QGraphicsView>
#include <QKeyEvent>

namespace OgreRenderer
{

    class QOgreUIView : public QGraphicsView
    {

    Q_OBJECT
    Q_PROPERTY(bool dirty_ WRITE setDirty READ isDirty)

    public:
        QOgreUIView (QWidget *parent, QGraphicsScene *scene);
        virtual ~QOgreUIView ();

        void SetWorldView(QOgreWorldView *view);
        void SetScene(QGraphicsScene *new_scene);
        void InitializeWorldView(int width, int height);
        
        Ogre::RenderWindow *CreateRenderWindow (const std::string &name, int width, int height, int left, int top, bool fullscreen);

    public slots:
        void setDirty(bool dirty) { dirty_ = dirty; }
        bool isDirty() { return dirty_; }

    protected:
        void keyPressEvent (QKeyEvent *e);
        void resizeEvent (QResizeEvent *e);

    private:
        void Initialize_ ();

        Ogre::RenderWindow  *win_;
        QOgreWorldView *view_;
        bool dirty_;

    private slots:
        void SceneChange();

    signals:
        void ConsoleToggleRequest();
        void PythonRestartRequest();
        
        void ViewKeyPressed(QKeyEvent *key_event);
    };

} 

#endif // incl_OgreRenderer_QOgreUIView_h
