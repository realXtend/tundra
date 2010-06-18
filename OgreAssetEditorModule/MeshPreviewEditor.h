// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_OgreAssetEditorModule_MeshPreviewEditor_h
#define incl_OgreAssetEditorModule_MeshPreviewEditor_h

#include <RexTypes.h>
#include <QWidget>
#include <QLabel>
#include <QImage>
#include <UiModule.h>
#include <QOgreWorldView.h>
#include "QOgreUIView.h"

#include <boost/shared_ptr.hpp>

QT_BEGIN_NAMESPACE
class QPushButton;
QT_END_NAMESPACE

namespace Foundation
{
    class Framework;
    class AssetInterface;
    typedef boost::shared_ptr<AssetInterface> AssetPtr;
}

namespace Resource
{
    namespace Events
    {
        class ResourceReady;
    }
}


namespace Naali
{
    //! Label is used to display the mesh in image format.
    class MeshPreviewLabel: public QLabel
    {
        Q_OBJECT
    public:
        MeshPreviewLabel(QWidget *parent = 0, Qt::WindowFlags flags = 0);
        virtual ~MeshPreviewLabel();
    signals:
        void sendMouseEvent(QMouseEvent *event, bool both);
    
    protected:
         void mouseMoveEvent(QMouseEvent *event);
         void mousePressEvent(QMouseEvent* ev);
         void mouseReleaseEvent(QMouseEvent* ev);
        
    private:
         bool leftPressed_;
         bool rightPressed_;
    };

    //! AudioPreviewEditor is used to play different audioclips from the inventory and show audio info.
    class MeshPreviewEditor: public QWidget
    {
        Q_OBJECT
    public:

        MeshPreviewEditor(Foundation::Framework *framework,
                           const QString &inventory_id,
                           const asset_type_t &asset_type,
                           const QString &name, 
                           const QString &assetID,
                           QWidget *parent = 0);
        virtual ~MeshPreviewEditor();

        //void HandleAssetReady(Foundation::AssetPtr asset);

        void HandleResouceReady(Resource::Events::ResourceReady *res);
        void RequestMeshAsset(const QString &asset_id);
        QImage ConvertToQImage(const u8 *raw_image_data, int width, int height, int channels);

    public slots:
        /// Close the window.
        void Closed();
        void Update();
        void MouseEvent(QMouseEvent* event, bool both);

    signals:
        /// This signal is emitted when the editor is closed.
        void Closed(const QString &inventory_id, asset_type_t asset_type);

    private:
       
        void InitializeEditorWidget();
       

        Foundation::Framework *framework_;
        asset_type_t assetType_;
        QString inventoryId_;

        QWidget     *mainWidget_;
        QPushButton *okButton_;
        QString assetId_;
        request_tag_t request_tag_;
        UiServices::UiProxyWidget *proxy_; 
        QPointF lastPos_;
        double camAlphaAngle_;
        double camPsiAngle_;
        QString mesh_id_;
        double val;
        double moveX_;
        double moveY_;
    };
}

#endif