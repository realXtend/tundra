// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DirectoryView.h"

#include <QUiLoader>
#include <QFile>
#include <QTreeView>
#include <QDirModel>
#include <QHeaderView>
#include <QVBoxLayout>

namespace QtUI
{

DirectoryView::DirectoryView(QObject *target, const char* slot, QWidget *parent) :
    QWidget(parent),
    //QTreeView(0),
    fileModel_(0)
{
    // Create widget from ui file
    QUiLoader loader;
    QFile uiFile("./data/ui/fileview.ui");
    widget_= loader.load(&uiFile, 0);
    widget_->resize(300, 350);
    uiFile.close();

    QWidget *w = widget_->findChild<QWidget *>("widgetFileView");
    widget_->setParent(this);
    w->setParent(widget_);
    QVBoxLayout *layout = widget_->findChild<QVBoxLayout *>("verticalLayout_2");
    layout->addWidget(w);

    treeView_ = new QTreeView(w);
    ///\todo The usage of QDirModel is not recommended anymore. The QFileSystemModel class is a more performant alternative.
    fileModel_ = new QDirModel();
    fileModel_->setSorting(QDir::DirsFirst | QDir::Type);
    treeView_->setModel(fileModel_);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    treeView_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    ///\todo Filters
    /*
    QStringList filters;
    filters << "*.mesh";
    filters << "*.material";
    filters << "*.particle";
    filters << "*.skeleton";
    filters << "*.xml";
    filters << "*.jpg";
    filters << "*.png";
    filters << "*.bmp";
    fileModel_->setNameFilters(filters);
    */

//    setRootIndex(fileModel_->index(QDircurrentPath()));
//    setCurrentIndex(fileModel_->index(QDir::currentPath()));
    treeView_->setAlternatingRowColors(true);
    treeView_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    treeView_->setSelectionMode(QTreeView::ExtendedSelection);
    treeView_->setSelectionBehavior(QTreeView::SelectItems);
//    setRootIsDecorated(true);
//    setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::SelectedClicked);

    // Set up header: Resize to contents, move "Type" before "File Size".
    treeView_->header()->setResizeMode(QHeaderView::ResizeToContents);
    treeView_->header()->moveSection(1, 2);

    connect(treeView_, SIGNAL(activated(const QModelIndex &)), this, SLOT(Activated())), 
    connect(treeView_, SIGNAL(expanded(const QModelIndex &)), this, SLOT(Resize()));
    connect(this, SIGNAL(FilesChosen(const QStringList &)), target, slot);

    setWindowTitle("Open");

    setAttribute(Qt::WA_DeleteOnClose,true);

    //treeView_->resize(299, 349);
    show();
    adjustSize();
}

//virtual
DirectoryView::~DirectoryView()
{
    SAFE_DELETE(fileModel_);
}

void DirectoryView::Activated()
{
    QStringList filenames;

    QModelIndexList indexes = treeView_->selectionModel()->selection().indexes();
    QListIterator<QModelIndex> it(indexes);
    while(it.hasNext())
    {
        QModelIndex index = it.next();
        if (fileModel_->isDir(index))
            continue;

        filenames << fileModel_->filePath(index);
    }

    emit FilesChosen(filenames);

    close();
}

void DirectoryView::InitUi()
{

}

}
