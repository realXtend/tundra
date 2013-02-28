// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <QVector>

#include <Ogre.h>

namespace OgreRenderer
{
    class Renderer;
}

namespace CAVEStereo
{
    class ExternalRenderWindow;
    class StereoManager;
    class StereoWidget;
    class CAVEStereoModule;

    class StereoController : public QObject
    {
        Q_OBJECT

    public:
        StereoController(OgreRenderer::Renderer *r, CAVEStereoModule* mod);
        virtual ~StereoController();

    public slots:
        void InitializeUi();
        void EnableStereo(QString& tech_type, qreal eye_dist, qreal focal_l, qreal offset, qreal scrn_width);
        void DisableStereo();
        void ChangeColorLeft(qreal r, qreal g, qreal b);
        void ChangeColorRight(qreal r, qreal g, qreal b);
        StereoWidget* GetStereoWidget() const;
        void StereoFlip();
        void TakeScreenshots(QString path, QString filename);

    private:
        void ChangeShaderColour(qreal r, qreal g, qreal b,const Ogre::String& name);
        Ogre::RenderWindow* CreateRenderWindow(const std::string &name,  int width, int height, int left, int top, bool fullscreen);

        QVector<Ogre::RenderWindow*> getRenderWindows();
        QMap<QString,StereoManager*> stereo_views_;
        QVector<ExternalRenderWindow*> windows_to_dispose_;
        OgreRenderer::Renderer* renderer_;
        CAVEStereoModule *module_;
        StereoWidget* settings_widget_;
        
        int number_of_views_;
        QString prefix_;
        bool flip_;
    };
}
