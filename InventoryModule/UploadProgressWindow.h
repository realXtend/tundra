// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   ItemPropertiesWindow.h
 *  @brief  Item properties window. Shows basic information about inventory item and the asset
 *          it's referencing to.
 */

#ifndef incl_InventoryModule_UploadProgressWindow_h
#define incl_InventoryModule_UploadProgressWindow_h

#include <QWidget>

namespace UiServices
{
    class UiProxyWidget;
}

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QProgressBar;
class QLabel;
QT_END_NAMESPACE

namespace Inventory
{
    class InventoryModule;

    class UploadProgressWindow : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor
        /// @param owner Inventory module.
        /// @param parent Parent widget.
        UploadProgressWindow(InventoryModule *owner, QWidget *parent = 0);

        /// Destructor.
        ~UploadProgressWindow();

    public slots:
        ///
        void OpenUploadProgress(size_t file_count);

        ///
        void UploadStarted(const QString &filename);

        ///
        void CloseUploadProgress();

    private:
        /// Inventory module
        InventoryModule *owner_;

        /// Main widget.
        QWidget *mainWidget_;

        /// Proxy widget.
        UiServices::UiProxyWidget *proxyWidget_;

        /// Layout.
        QVBoxLayout *layout_;

        /// Progress bar.
        QProgressBar *progressBar_;

        /// File number label.
        QLabel *labelFileNumber_;

        /// Upload count.
        size_t uploadCount_;
    };
}

#endif
