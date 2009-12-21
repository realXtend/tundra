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
        DirectoryView(QObject *target, const char* slot, QWidget *parent = 0);
        virtual ~DirectoryView();

    signals:
        void FilesChosen(const QStringList &);

    private slots:
        void Activated();

    private:
        void InitUi();
        QWidget *widget_;
        QTreeView *treeView_;
        QDirModel *fileModel_;
    };
}

#endif
