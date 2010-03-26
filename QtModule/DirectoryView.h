// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_QtModule_DirectoryView_h
#define incl_QtModule_DirectoryView_h

#include "QtModuleApi.h"

//#include <QTreeView>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QWidget;
class QTreeView;
class QDirModel;
QT_END_NAMESPACE

namespace QtUI
{
    //class QT_MODULE_API DirectoryView : public QTreeView
    class QT_MODULE_API DirectoryView : public QWidget
    {
        Q_OBJECT

    public:
        /// Constructor.
        /// @param target Target object (and owner of the slot).
        /// @param slot Slot.
        /// @param Parent.
        DirectoryView(QObject *target, const char* slot, QWidget *parent = 0);

        /// Destructor.
        virtual ~DirectoryView();

    signals:
        /// Emitted when files are chosen.
        void FilesChosen(const QStringList &);

    private slots:
        /// This slot emit the FilesChosen signal when user activates (double-click or enter) files on the directory view.
        void Activated();

    private:
        /// Initializes the UI.
        void InitUi();

        /// Main widget.
        QWidget *widget_;

        /// Directory tree view.
        QTreeView *treeView_;

        /// Directory model.
        QDirModel *fileModel_;
    };
}

#endif
