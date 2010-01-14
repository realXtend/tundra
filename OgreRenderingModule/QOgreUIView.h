// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreRenderer_QOgreUIView_h
#define incl_OgreRenderer_QOgreUIView_h

#include "StableHeaders.h"

#include "QOgreWorldView.h"

#include <Ogre.h>
#include <QGraphicsView>

namespace OgreRenderer
{

    class QOgreUIView : public QGraphicsView
    {

    Q_OBJECT
    Q_PROPERTY(bool dirty_ WRITE setDirty READ isDirty)

    public:
        QOgreUIView ();
        QOgreUIView (QWidget *parent, QGraphicsScene *scene);
        virtual ~QOgreUIView ();

        void SetWorldView(QOgreWorldView *view);
        void InitializeWorldView(int width, int height); 

        Ogre::RenderWindow *CreateRenderWindow (const std::string &name, int width, int height, int left, int top, bool fullscreen);

    public slots:
        void setDirty(bool dirty) { dirty_ = dirty; }
        bool isDirty() { return dirty_; }

    protected:
        void resizeEvent (QResizeEvent *e);

    private:
        void Initialize_ ();

        Ogre::RenderWindow  *win_;
        QOgreWorldView *view_;
        bool dirty_;

    private slots:
        void SceneChange();

    };

} 

#endif // incl_OgreRenderer_QOgreUIView_h
