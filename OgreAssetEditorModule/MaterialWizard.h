// For conditions of distribution and use, see copyright notice in license.txt

/**
 *  @file   MaterialWizard.h
 *  @brief  Utitility tool for choosing right material script for your purpose from
 *          the Naali material script template library.
 */

#ifndef incl_OgreAssetEditorModule_MaterialWizard_h
#define incl_OgreAssetEditorModule_MaterialWizard_h

#include <QObject>

namespace Foundation
{
    class Framework;
}

namespace UiServices
{
    class UiProxyWidget;
}

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace OgreAssetEditor
{
    class MaterialWizard : public QObject
    {
        Q_OBJECT

    public:
        /// Consturctor.
        /// @param framework Framework pointer.
        MaterialWizard(Foundation::Framework *framework);

        /// Destructor.
        ~MaterialWizard();

    private:
        /// Initializes the UI.
        void InitWindow();

        /// Framework pointer.
        Foundation::Framework *framework_;

        /// Proxy widget for the UI.
        UiServices::UiProxyWidget *proxyWidget_;

        /// The window main widget.
        QWidget *mainWidget_;
    };
}

#endif
