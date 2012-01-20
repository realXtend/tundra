/**
 *  For conditions of distribution and use, see copyright notice in LICENSE
 *
 *  @file   AddContentWindow.cpp
 *  @brief  Window for adding new content and uploading assets.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "AddContentWindow.h"
#include "SceneStructureModule.h"
#include "TreeWidgetUtils.h"
#include "SupportedFileTypes.h"

#include "UiAPI.h"
#include "UiMainWindow.h"
#include "Framework.h"
#include "AssetAPI.h"
#include "IAssetStorage.h"
#include "IAssetUploadTransfer.h"
#include "SceneDesc.h"
#include "Scene.h"
#include "SceneImporter.h"
#include "Transform.h"
#include "LoggingFunctions.h"
#include "CoreException.h"
#include "ConfigAPI.h"

#include "MemoryLeakCheck.h"

typedef QMap<QString, QString> RefMap;

/// @cond PRIVATE
void ReplaceReferences(QByteArray &material, const RefMap &refs)
{
    QString matString(material);
    QStringList lines = matString.split("\n");
    for(int i = 0; i < lines.size(); ++i)
    {
        int idx = lines[i].indexOf("texture ");
        if (idx != -1)
        {
            QString texName = lines[i].mid(idx + 8).trimmed();
            texName = AssetAPI::ExtractFilenameFromAssetRef(texName); ///\todo This line is wrong and should be removed.
            // There is a problem in the refmap that it doesn't store the original assetrefs, but stores the filenames
            // without paths. Therefore we need to do the comparison here also without paths.
            RefMap::const_iterator it = refs.find(texName);
            if (it != refs.end())
            {
                lines[i] = "texture " + it.value();
                for(int spaceIdx = 0; spaceIdx < idx; ++spaceIdx)
                    lines[i].prepend(" ");
            }
        }
    }

    material = lines.join("\n").toAscii();
}
/// @endcond

// Entity tree widget column index enumeration.
const int cColumnEntityCreate = 0; ///< Create column index.
const int cColumnEntityId = 1; ///< ID column index.
const int cColumnEntityName = 2; ///< Name column index.

// Asset tree widget column index enumeration.
const int cColumnAssetUpload = 0; ///< Upload column index;
const int cColumnAssetTypeName = 1; ///< Type name column index.
const int cColumnAssetSourceName = 2; ///< Source name column index.
const int cColumnAssetSubname = 3; ///< Subname column index.
const int cColumnAssetDestName = 4; ///< Destination name column index.

/// Tree widget item representing an entity.
/// @cond PRIVATE
class EntityWidgetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param edesc Entity description. */
    explicit EntityWidgetItem(const EntityDesc &edesc) : desc(edesc)
    {
        setCheckState(cColumnEntityCreate, Qt::Checked);
        setText(cColumnEntityId, desc.id);
        setText(cColumnEntityName, desc.name);
    }

    /// QTreeWidgetItem override. Peforms case-insensitive comparison.
    bool operator <(const QTreeWidgetItem &rhs) const
    {
        int column = treeWidget()->sortColumn();
        if (column == cColumnEntityCreate)
            return checkState(column) < rhs.checkState(column);
        else if (column == cColumnEntityId)
            return text(column).toInt() < rhs.text(column).toInt();
        else
            return text(column).toLower() < rhs.text(column).toLower();
    }

    EntityDesc desc; ///< Entity description of the item.
};
/// @endcond

/// Tree widget item representing an asset.
/// @cond PRIVATE
class AssetWidgetItem : public QTreeWidgetItem
{
public:
    /// Constructor.
    /** @param adesc Asset description. */
    explicit AssetWidgetItem(const AssetDesc &adesc) : desc(adesc)
    {
        RewriteText();
    }

    /// QTreeWidgetItem override. Peforms case-insensitive comparison.
    bool operator <(const QTreeWidgetItem &rhs) const
    {
        int column = treeWidget()->sortColumn();
        if (column == cColumnAssetUpload)
            return checkState(column) < rhs.checkState(column);
        else
            return text(column).toLower() < rhs.text(column).toLower();
    }

    /// Rewrites the items visible text accordingly to the asset description the item owns.
    void RewriteText()
    {
        setCheckState(cColumnAssetUpload, Qt::Checked);
        setText(cColumnAssetTypeName, desc.typeName);
        setText(cColumnAssetSourceName, desc.source);
        setText(cColumnAssetSubname, desc.subname);
        setText(cColumnAssetDestName, desc.destinationName);
    }

    AssetDesc desc; ///< Asset description of the item.
};
/// @endcond PRIVATE

/// A special case identifier for using the default storage.
const char *cDefaultStorage = "DefaultStorage";
/// A special case identifier for not altering asset refs when uploading assets.
const char *cDoNotAlterAssetReferences = "DoNotAlterAssetReferences";
// Used for saving the recently used storage to config.
const char *cRecentStorageSetting = "recent strorage";
const char *cAddContentDialogSetting = "AddContentDialog";

AddContentWindow::AddContentWindow(Framework *fw, const ScenePtr &dest, QWidget *parent) :
    QWidget(parent),
    framework(fw),
    scene(dest),
    parentEntities_(0),
    parentAssets_(0),
    position(float3::zero),
    progressStep_(0),
    failedUploads_(0),
    successfulUploads(0),
    totalUploads_(0)
{
    setWindowModality(Qt::ApplicationModal/*Qt::WindowModal*/);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle("Entity And Asset Import");

    // If not parent was given, lets make our main window the parent
    // and set the window flags to match the other tundra dialogs/windows
    if (!parent && framework->Ui()->MainWindow())
    {
        setParent(framework->Ui()->MainWindow());
        setWindowFlags(Qt::Tool);
    }
    else if (parent && (parent == framework->Ui()->MainWindow()))
        setWindowFlags(Qt::Tool);

    QPixmap nullIcon(16,16); // do a null icon to hide the default ugly one
    nullIcon.fill(Qt::transparent);
    setWindowIcon(nullIcon);

    QString widgetStyle = "QProgressBar { border: 1px solid grey; border-radius: 0px; text-align: center; background-color: rgb(244, 244, 244); }" 
        "QProgressBar::chunk { background-color: qlineargradient(spread:pad, x1:0.028, y1:1, x2:0.972, y2:1, stop:0 rgba(194, 194, 194, 100), stop:1 rgba(115, 115, 115, 100)); }";
    setStyleSheet(widgetStyle);

    QFont titleFont("Arial", 11);
    titleFont.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);

    // Entities ui
    parentEntities_ = new QWidget();
    QVBoxLayout *entitiesLayout = new QVBoxLayout();
    entitiesLayout->setContentsMargins(0, 0, 0, 0);
    parentEntities_->setLayout(entitiesLayout);

    QLabel *entityLabel = new QLabel(tr("Entity Creation"));
    entityLabel->setFont(titleFont);
    entityLabel->setStyleSheet("color: rgb(63, 63, 63);");

    entityTreeWidget = new QTreeWidget;
    entityTreeWidget->setColumnCount(3);
    entityTreeWidget->setHeaderLabels(QStringList(QStringList() << tr("Create") << tr("ID") << tr("Name")));
    entityTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);
    entityTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPushButton *selectAllEntitiesButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllEntitiesButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *entityButtonsLayout = new QHBoxLayout;
    QSpacerItem *entityButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *middleSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *entitiesProgressLayout = new QVBoxLayout();
    entityStatusLabel = new QLabel();
    entityProgressBar = new QProgressBar();
    entityProgressBar->setFixedHeight(20);

    entitiesLayout->addWidget(entityLabel);
    entitiesLayout->addLayout(entitiesProgressLayout);
    entitiesLayout->addWidget(entityTreeWidget);
    entitiesProgressLayout->addWidget(entityStatusLabel);
    entitiesProgressLayout->addWidget(entityProgressBar);
    entityButtonsLayout->addWidget(selectAllEntitiesButton);
    entityButtonsLayout->addWidget(deselectAllEntitiesButton);
    entityButtonsLayout->addSpacerItem(entityButtonSpacer);
    //entitiesLayout->insertLayout(-1, entitiesProgressLayout);
    entitiesLayout->insertLayout(-1, entityButtonsLayout);
    entitiesLayout->insertSpacerItem(-1, middleSpacer);
    layout->addWidget(parentEntities_);

    // Assets ui
    parentAssets_ = new QWidget();
    QVBoxLayout *assetsLayout = new QVBoxLayout();
    assetsLayout->setContentsMargins(0, 0, 0, 0);
    parentAssets_->setLayout(assetsLayout);

    QLabel *assetLabel = new QLabel(tr("Asset Uploads"));
    assetLabel->setFont(titleFont);
    assetLabel->setStyleSheet("color: rgb(63, 63, 63);");

    assetTreeWidget = new QTreeWidget;
    assetTreeWidget->setColumnCount(5);
    QStringList labels;
    labels << tr("Upload") << tr("Type") << tr("Source name") << tr("Source subname") << tr("Destination name");
    assetTreeWidget->setHeaderLabels(labels);
    assetTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPushButton *selectAllAssetsButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllAssetsButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *assetButtonsLayout = new QHBoxLayout;
    QSpacerItem *assetButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel *storageLabel = new QLabel(tr("Asset storage:"));
    storageComboBox = new QComboBox;

    QVBoxLayout *uploadLayout = new QVBoxLayout();
    uploadStatusLabel = new QLabel();
    uploadProgressBar = new QProgressBar();
    uploadProgressBar->setFixedHeight(20);

    GenerateStorageComboBoxContents();

    uploadLayout->addWidget(uploadStatusLabel);
    uploadLayout->addWidget(uploadProgressBar);

    assetsLayout->addWidget(assetLabel);
    assetsLayout->addLayout(uploadLayout);
    assetsLayout->addWidget(assetTreeWidget);
    //assetsLayout->addLayout(uploadLayout);
    assetButtonsLayout->addWidget(selectAllAssetsButton);
    assetButtonsLayout->addWidget(deselectAllAssetsButton);
    assetButtonsLayout->addSpacerItem(assetButtonSpacer);
    assetButtonsLayout->addWidget(storageLabel);
    assetButtonsLayout->addWidget(storageComboBox);
    assetsLayout->insertLayout(-1, assetButtonsLayout);
    layout->addWidget(parentAssets_);

    uploadStatusLabel->hide();
    uploadProgressBar->hide();
    entityStatusLabel->hide();
    entityProgressBar->hide();

    // General controls
    addContentButton = new QPushButton(tr("Add content"));
    cancelButton = new QPushButton(tr("Cancel"));

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    QSpacerItem *buttonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonsLayout->addSpacerItem(buttonSpacer);
    buttonsLayout->addWidget(addContentButton);
    buttonsLayout->addWidget(cancelButton);
    layout->insertLayout(-1, buttonsLayout);

    connect(addContentButton, SIGNAL(clicked()), SLOT(AddContent()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(Close()));
    connect(selectAllEntitiesButton, SIGNAL(clicked()), SLOT(SelectAllEntities()));
    connect(deselectAllEntitiesButton, SIGNAL(clicked()), SLOT(DeselectAllEntities()));
    connect(selectAllAssetsButton, SIGNAL(clicked()), SLOT(SelectAllAssets()));
    connect(deselectAllAssetsButton, SIGNAL(clicked()), SLOT(DeselectAllAssets()));
    connect(assetTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(CheckIfColumnIsEditable(QTreeWidgetItem *, int)));
    connect(storageComboBox, SIGNAL(currentIndexChanged(int)), SLOT(RewriteDestinationNames()));

    // If we saved recently used storage, and the storage exists in the current combo box, set it as current index.
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, cAddContentDialogSetting, cRecentStorageSetting);
    QString storageName = framework->Config()->Get(configData).toString();
    if (!storageName.isEmpty())
        for(int i = 0; i < storageComboBox->count(); ++i)
            if (storageComboBox->itemData(i) == storageName)
            {
                storageComboBox->setCurrentIndex(i);
                break;
            }
}

AddContentWindow::~AddContentWindow()
{
    // Disable ResizeToContents, Qt goes sometimes into eternal loop after
    // ~AddContentWindow() if we have lots (hudreds or thousands) of items.
    entityTreeWidget->header()->setResizeMode(QHeaderView::Interactive);
    entityTreeWidget->setSortingEnabled(false);
    assetTreeWidget->header()->setResizeMode(QHeaderView::Interactive);
    assetTreeWidget->setSortingEnabled(false);

    QTreeWidgetItemIterator it(entityTreeWidget);
    while(*it)
    {
        QTreeWidgetItem *item = *it;
        SAFE_DELETE(item);
        ++it;
    }

    QTreeWidgetItemIterator it2(assetTreeWidget);
    while(*it2)
    {
        QTreeWidgetItem *item = *it2;
        SAFE_DELETE(item);
        ++it2;
    }
}

void AddContentWindow::showEvent(QShowEvent *e)
{
    CenterToMainWindow();
    QWidget::showEvent(e);
}

void AddContentWindow::AddDescription(const SceneDesc &desc)
{
    sceneDescs.clear();
    sceneDescs.append(desc);
    AddEntities(desc.entities);
    AddAssets(desc, desc.assets);
    resize(1,1);
}

void AddContentWindow::AddDescription(const QList<SceneDesc> &descs)
{
    sceneDescs.clear();
    sceneDescs = descs;
    foreach(const SceneDesc & desc, sceneDescs)
    {
        AddEntities(desc.entities);
        AddAssets(desc, desc.assets);
    }
    resize(1,1);
}

void AddContentWindow::AddAssets(const QStringList &fileNames)
{
    assetTreeWidget->setSortingEnabled(false);

    SceneDesc desc;
    foreach(const QString &file, fileNames)
    {
        AssetDesc ad;
        ad.source = file;
        ad.dataInMemory = false;
        QString type = AssetAPI::GetResourceTypeFromAssetRef(file);
        ad.typeName = type.isEmpty() ? "Binary" : type;
        ad.destinationName = QFileInfo(file).fileName();
        desc.assets[qMakePair(ad.source, ad.subname)]= ad;
    }

    sceneDescs.clear();
    sceneDescs.append(desc);

    AddAssets(desc, desc.assets);
}

void AddContentWindow::AddEntities(const QList<EntityDesc> &entityDescs)
{
    if (entityDescs.empty())
    {
        SetEntitiesVisible(false);
        return;
    }

    // Disable sorting while we insert items.
    entityTreeWidget->setSortingEnabled(false);

    foreach(const EntityDesc &e, entityDescs)
    {
        EntityWidgetItem *eItem = new EntityWidgetItem(e);
        entityTreeWidget->addTopLevelItem(eItem);

        // Not showing components nor attributes in the UI for now.
/*
        foreach(ComponentDesc c, e.components)
        {
            QTreeWidgetItem *cItem = new QTreeWidgetItem;
            cItem->setText(0, c.typeName + " " + c.name);
            eItem->addChild(cItem);
*/

            // Gather non-empty asset references. They're shown in their own tree widget.
/*
            foreach(AttributeDesc a, c.attributes)
                if (a.typeName == "assetreference" && !a.value.isEmpty())
                    assetRefs.insert(a);
        }
*/
    }

    entityTreeWidget->setSortingEnabled(true);

    // Set a minimum height for our treeview
    int fullHeight = entityTreeWidget->header()->height();
    fullHeight += (entityTreeWidget->sizeHintForRow(0)+5) * entityTreeWidget->model()->rowCount();
    int halfDeskHeight = QApplication::desktop()->screenGeometry().height()/2;
    if (fullHeight < halfDeskHeight-300)
        entityTreeWidget->setMinimumHeight(fullHeight);
    else
        entityTreeWidget->setMinimumHeight(halfDeskHeight - 300);
}

void AddContentWindow::AddAssets(const SceneDesc &sceneDesc, const SceneDesc::AssetMap &assetDescs)
{
    assetTreeWidget->setSortingEnabled(false);

    foreach(const AssetDesc &a, assetDescs)
    {
        AssetWidgetItem *aItem = new AssetWidgetItem(a);
        assetTreeWidget->addTopLevelItem(aItem);

        QString basePath = QFileInfo(sceneDesc.filename).dir().path();
        QString outFilePath;
        AssetAPI::FileQueryResult res = framework->Asset()->ResolveLocalAssetPath(a.source, basePath, outFilePath);

        /*if (res == AssetAPI::FileQueryLocalFileFound)
        {
            // If file is found locally rewrite the source for asset desc.
            QList<AssetDesc>::iterator ai = qFind(sceneDesc.assets.begin(), sceneDesc.assets.end(), aItem->desc);
            if (ai != sceneDesc.assets.end())
            {
                aItem->desc.source = outFilePath;
                aItem->RewriteText();
                (*ai).source = outFilePath;
            }
        }
        */
        if ((a.typeName == "material" && a.data.isEmpty()) || res == AssetAPI::FileQueryLocalFileMissing)
        {
            // File not found, mark the item red and disable it.
            aItem->setBackgroundColor(cColumnAssetSourceName, QColor(255,0,0,200));
            aItem->setTextColor(cColumnAssetSourceName, Qt::white);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);
        }
        else if (res == AssetAPI::FileQueryExternalFile)
        {
            // External reference, mark the item gray and disable it.
            aItem->setBackgroundColor(cColumnAssetSourceName, Qt::gray);
            aItem->setTextColor(cColumnAssetSourceName, Qt::white);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);
        }
    }

    RewriteDestinationNames();

    // Enable sorting, resize header sections to contents.
    assetTreeWidget->setSortingEnabled(true);
    assetTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    // Set a minimum height for our treeview
    int fullHeight = assetTreeWidget->header()->height();
    fullHeight += (assetTreeWidget->sizeHintForRow(0)+5) * assetTreeWidget->model()->rowCount();

    int halfDeskHeight = QApplication::desktop()->screenGeometry().height()/2;
    assetTreeWidget->setMinimumHeight(fullHeight < halfDeskHeight-50 ? fullHeight : halfDeskHeight-100);

    // Set the windows minumum width from assets tree view
    int minWidth = 10;
    for(int i=0; i<assetTreeWidget->columnCount(); i++)
        minWidth += assetTreeWidget->columnWidth(i);
    setMinimumWidth(minWidth);

    // Sort asset items initially so that erroneous are first
    assetTreeWidget->sortItems(cColumnAssetUpload, Qt::AscendingOrder);
}

void AddContentWindow::RewriteAssetReferences(SceneDesc &sceneDesc, const AssetStoragePtr &dest, bool useDefaultStorage)
{
    QString path = QFileInfo(sceneDesc.filename).dir().path();
    QList<SceneDesc::AssetMapKey> keysWithSubname;
    foreach(const SceneDesc::AssetMapKey &key, sceneDesc.assets.keys())
        if (!key.second.isEmpty())
            keysWithSubname.append(key);

    QMutableListIterator<EntityDesc > edIt(sceneDesc.entities);
    while(edIt.hasNext())
    {
        QMutableListIterator<ComponentDesc> cdIt(edIt.next().components);
        while(cdIt.hasNext())
        {
            QMutableListIterator<AttributeDesc> adIt(cdIt.next().attributes);
            while(adIt.hasNext())
            {
                adIt.next();
                if (adIt.value().typeName == "assetreference" || adIt.value().typeName == "assetreferencelist")
                {
                    QStringList values = adIt.value().value.split(";");
                    QStringList newValues;
                    foreach(QString value, values)
                    {
                        if (value.isEmpty())
                            continue;

                        QString subname;

                        if (!keysWithSubname.isEmpty())
                        {
                            ///\todo This string manipulation/crafting doesn't work for .zip files, only for materials and COLLADA files
                            int slashIdx = value.lastIndexOf("/");
                            int dotIdx = value.lastIndexOf(".");
                            QString str = value.mid(slashIdx + 1, dotIdx - slashIdx - 1);

                            foreach(const SceneDesc::AssetMapKey &key, keysWithSubname)
                                if (value == key.first && str == key.second)
                                {
                                    value = key.first;
                                    subname = key.second;
                                    break;
                                }
                        }
                        else
                        {
                            QString ref = AssetAPI::ExtractFilenameFromAssetRef(value);
                            ///\todo Perf hit, find a better way and remove this.
                            framework->Asset()->ResolveLocalAssetPath(ref, path, value);
                        }

                        SceneDesc::AssetMapKey key = qMakePair(value, subname);
                        if (sceneDesc.assets.contains(key))
                        {
                            if (useDefaultStorage)
                                newValues << AssetAPI::ExtractFilenameFromAssetRef(sceneDesc.assets[key].destinationName);
                            else
                                newValues << dest->GetFullAssetURL(sceneDesc.assets[key].destinationName);
                        }
                    }

                    if (!newValues.isEmpty())
                        adIt.value().value = newValues.join(";");
                    // After the above lines the asset reference attribute descs do not point to the original source assets.
                }
            }
        }
    }
}

QString AddContentWindow::CurrentStorageName() const
{
    return storageComboBox->itemData(storageComboBox->currentIndex()).toString();
}

void AddContentWindow::SelectAllEntities()
{
    TreeWidgetSetCheckStateForAllItems(entityTreeWidget, cColumnEntityCreate, Qt::Checked);
}

void AddContentWindow::DeselectAllEntities()
{
    TreeWidgetSetCheckStateForAllItems(entityTreeWidget, cColumnEntityCreate, Qt::Unchecked);
}

void AddContentWindow::SelectAllAssets()
{
    TreeWidgetSetCheckStateForAllItems(assetTreeWidget, cColumnAssetUpload, Qt::Checked);
}

void AddContentWindow::DeselectAllAssets()
{
    TreeWidgetSetCheckStateForAllItems(assetTreeWidget, cColumnAssetUpload, Qt::Unchecked);
}

void AddContentWindow::AddContent()
{
    uploadStatusLabel->setText("");
    uploadProgressBar->setValue(0);
    entityStatusLabel->setText(tr("Waiting for asset uploads to finish..."));
    entityProgressBar->setValue(0);
    bool uploadFailed = false;
    if (CreateNewDesctiption())
    {
        // If no uploads are queued then AddEntities will be called automatically
        if (UploadAssets())
        {
            if (totalUploads_ > 0)
            {
                uploadStatusLabel->show();
                uploadProgressBar->show();
            }
            if (!filteredDesc.entities.empty())
            {
                entityStatusLabel->show();
                entityProgressBar->show();
            }

            addContentButton->setEnabled(false);
            storageComboBox->setEnabled(false);

            // Upload successful, save most recently used storage to config.
            ConfigData configData(ConfigAPI::FILE_FRAMEWORK, cAddContentDialogSetting, cRecentStorageSetting, CurrentStorageName());
            framework->Config()->Set(configData);
        }
        else
            uploadFailed = true;
    }
    else
        uploadFailed = true;

    if (uploadFailed)
        QMessageBox::critical(this, tr("Uploading"), tr("Starting uploads failed"));
}

bool AddContentWindow::CreateNewDesctiption()
{
    QString storageName = CurrentStorageName();
    AssetStoragePtr dest;
    if (storageName != cDoNotAlterAssetReferences)
    {
        dest = storageName == cDefaultStorage ? framework->Asset()->GetDefaultAssetStorage() : framework->Asset()->GetAssetStorageByName(storageName);
        if (!dest)
        {
            LogError("AddContentWindow::CreateNewDesctiption: Could not retrieve asset storage " + storageName + ".");
            GenerateStorageComboBoxContents(); // Regenerate storage combo box items to make sure that we're up-to-date.
            return false;
        }
    }

    if (dest && !dest->Writable())
    {
        QString errorMsg = tr("Read-only storage %1 cannot be used for asset upload.").arg(dest->Name());
        uploadStatusLabel->setText(errorMsg);
        uploadStatusLabel->show();
        LogError("AddContentWindow::CreateNewDesctiption: " + errorMsg);
        return false;
    }

    // Filter which entities will be created and which assets will be uploaded.
    filteredDesc = SceneDesc();
    filteredDesc.filename = sceneDescs.last().filename; ///< @todo For now, we need to now the filename if it's a Ogre .scene.
    filteredDesc.viewEnabled = sceneDescs.last().viewEnabled;
    foreach(const SceneDesc &desc, sceneDescs)
    {
        filteredDesc.entities.append(desc.entities);
        filteredDesc.assets.unite(desc.assets);
    }

    QTreeWidgetItemIterator eit(entityTreeWidget);
    while(*eit)
    {
        EntityWidgetItem *eitem = dynamic_cast<EntityWidgetItem *>(*eit);
        if (eitem && eitem->checkState(cColumnEntityCreate) == Qt::Unchecked)
        {
            QList<EntityDesc>::const_iterator ei = qFind(filteredDesc.entities, eitem->desc);
            if (ei != filteredDesc.entities.end())
                filteredDesc.entities.removeOne(*ei);
        }

        ++eit;
    }

    // Rewrite components' asset refs, if storage selected.
    bool rewrite = storageName != cDoNotAlterAssetReferences;
    if (rewrite)
    {
        bool useDefault = storageName == cDefaultStorage;
        RewriteAssetReferences(filteredDesc, dest, useDefault);
    }

    RefMap refs;
    QTreeWidgetItemIterator ait(assetTreeWidget);
    while(*ait)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*ait);
        assert(aitem);
        if (aitem)
        {
            if (aitem->checkState(cColumnAssetUpload) == Qt::Unchecked || aitem->isDisabled())
            {
                bool removed = filteredDesc.assets.remove(qMakePair(aitem->desc.source, aitem->desc.subname));
                if (!removed)
                    LogDebug("Couldn't find and remove " + aitem->desc.source + "from asset map.");
            }
            else
            {
                // Add textures to special map for later use.
                ///\todo This logic will be removed in the future, as we need it generic for any types of assets.
                if (aitem->desc.typeName == "texture")
                {
                    int idx = aitem->desc.source.lastIndexOf("/");
                    refs[aitem->desc.source.mid(idx != -1 ? idx + 1 : 0).trimmed()] = aitem->desc.destinationName;
                }
            }
        }

        ++ait;
    }

    // Rewrite asset refs
    QMutableMapIterator<SceneDesc::AssetMapKey, AssetDesc> rewriteIt(filteredDesc.assets);
    while(rewriteIt.hasNext())
    {
        rewriteIt.next();
        if (rewriteIt.value().typeName.contains("material", Qt::CaseInsensitive))
            ///\todo This logic will be removed in the future, as we need it generic for any types of assets.
            ReplaceReferences(rewriteIt.value().data, refs);
    }

    return true;
}

bool AddContentWindow::UploadAssets()
{
    AssetStoragePtr dest;
    QString storageName = CurrentStorageName();
    bool doNotAlter = storageName == cDoNotAlterAssetReferences;
    if (!doNotAlter) // No upload if we don't touch asset refs
    {
        dest = storageName == cDefaultStorage ? framework->Asset()->GetDefaultAssetStorage() : dest = framework->Asset()->GetAssetStorageByName(storageName);
        if (!dest)
        {
            LogError("AddContentWindow::UploadAssets: Could not retrieve asset storage " + storageName + ".");
            GenerateStorageComboBoxContents(); // Regenerate storage combo box items to make sure that we're up-to-date.
            return false;
        }
    }

    if (dest && !dest->Writable())
    {
        QString errorMsg = tr("Read-only storage %1 cannot be used for asset upload.").arg(dest->Name());
        uploadStatusLabel->setText(errorMsg);
        uploadStatusLabel->show();
        LogError("AddContentWindow::UploadAssets: " + errorMsg);
        return false;
    }

    // Upload
    if (!filteredDesc.assets.empty())
    {
        LogDebug(QString("Starting uploading of %1 asset%2.").arg(filteredDesc.assets.size()).arg(filteredDesc.assets.size() == 1 ? "." : "s."));

        totalUploads_ = 0;
        progressStep_ = 0;
        failedUploads_ = 0;
        successfulUploads = 0;
        foreach(const AssetDesc &ad, filteredDesc.assets)
        {
            try
            {
                AssetUploadTransferPtr transfer;

                if (ad.dataInMemory)
                {
                    transfer = framework->Asset()->UploadAssetFromFileInMemory((const u8*)ad.data.data(), ad.data.size(), dest, ad.destinationName);
                }
                else
                {
                    transfer = framework->Asset()->UploadAssetFromFile(ad.source, dest, ad.destinationName);
                }

                if (transfer &&
                    connect(transfer.get(), SIGNAL(Completed(IAssetUploadTransfer *)),
                        SLOT(HandleUploadCompleted(IAssetUploadTransfer *)), Qt::UniqueConnection) &&
                    connect(transfer.get(), SIGNAL(Failed(IAssetUploadTransfer *)),
                        SLOT(HandleUploadFailed(IAssetUploadTransfer *)), Qt::UniqueConnection))
                {
                    totalUploads_++;
                }
            }
            catch(const Exception &e)
            {
                LogError(std::string(e.what()));
            }
        }
        if (totalUploads_ > 0)
            progressStep_ = 100 / totalUploads_;
        else
            progressStep_ = 100;
    }
    else
        AddEntities(); // if no uploads, then add entities now!
    return true;
}

void AddContentWindow::AddEntities()
{
    AssetStoragePtr dest;
    QString storageName = CurrentStorageName();
    bool doNotAlter = storageName == cDoNotAlterAssetReferences;
    if (!doNotAlter) // No upload if we don't touch asset refs
    {
        dest = storageName == cDefaultStorage ? framework->Asset()->GetDefaultAssetStorage() : dest = framework->Asset()->GetAssetStorageByName(storageName);
        if (!dest)
        {
            LogError("AddContentWindow::AddEntities: Could not retrieve asset storage " + storageName + ".");
            GenerateStorageComboBoxContents(); // Regenerate storage combo box items to make sure that we're up-to-date.
            return;
        }
    }

    ScenePtr destScene = scene.lock();
    if (!destScene)
    {
        LogError("AddContentWindow::AddEntities: No destination scene. Could not add entities!");
        return;
    }

    if (!filteredDesc.entities.empty())
    {
        QList<Entity *> entities;
        /// @todo When SceneImporter is fixed, use Scene::CreateContentFromSceneDesc for all file types.
        if (filteredDesc.filename.endsWith(cOgreSceneFileExtension, Qt::CaseInsensitive))
        {
            if (!dest)
            {
                LogError("AddContentWindow::AddEntities: Ogre .scene cannot be upload without destination asset storage.");
                return;
            }

            QString path = QFileInfo(filteredDesc.filename).dir().path();
            TundraLogic::SceneImporter importer(destScene);
            entities = importer.Import(filteredDesc.filename, path, Transform(), dest->BaseURL(), AttributeChange::Default, false/*clearScene*/, false);
        }
        else
        {
            entities = destScene->CreateContentFromSceneDesc(filteredDesc, false, AttributeChange::Default);
        }

        if (!entities.empty())
        {
            entityStatusLabel->setText(QString(tr("%1/%2 entities created successfully")).arg(entities.count()).arg(filteredDesc.entities.count()));
            if (position != float3::zero)
                SceneStructureModule::CentralizeEntitiesTo(position, entities);
        }
        else
        {
            QString errorMsg = tr("No entities were created, even if the intent was to create %1 .").arg(filteredDesc.entities.count());
            entityStatusLabel->setText(errorMsg);
            QMessageBox::warning(this, tr("Entity Creation"), errorMsg);
            return;
        }

        entityProgressBar->setValue(100);
    }
    else
        entityStatusLabel->setText(tr("No entities were selected for add"));

    cancelButton->setText(tr("Close"));
    addContentButton->setEnabled(true);
    storageComboBox->setEnabled(true);
}

void AddContentWindow::CenterToMainWindow()
{
    if (framework->Ui()->MainWindow())
    {
        QRect mainRect = framework->Ui()->MainWindow()->rect();
        QPoint mainPos = framework->Ui()->MainWindow()->pos();
        QPoint mainCenter = mainPos + mainRect.center();
        move(mainCenter.x()-width()/2, (mainCenter.y()-height()/2) >= 0 ? mainCenter.y()-height()/2 : 0);
    }
}

void AddContentWindow::SetEntitiesVisible(bool visible)
{
    parentEntities_->setVisible(visible);
    resize(width(), 200);
    CenterToMainWindow();
}

void AddContentWindow::SetAssetsVisible(bool visible)
{
    parentAssets_->setVisible(visible);
    resize(width(), 200);
    CenterToMainWindow();
}

void AddContentWindow::Close()
{
    AssetStoragePtr storage = framework->Asset()->GetAssetStorageByName(CurrentStorageName());
    QString currentStorageBaseUrl = storage ? storage->BaseURL() : "";
    emit Completed(entityProgressBar->value() > 0, currentStorageBaseUrl);
    close();
}

void AddContentWindow::CheckIfColumnIsEditable(QTreeWidgetItem *item, int column)
{
    if (column == cColumnAssetDestName)
        item->setFlags(item->flags() | Qt::ItemIsEditable);
    else
        item->setFlags(item->flags() & ~Qt::ItemIsEditable);
}

void AddContentWindow::GenerateStorageComboBoxContents()
{
    storageComboBox->clear();
    storageComboBox->addItem(tr("Default storage"), cDefaultStorage);
    storageComboBox->addItem(tr("Do not alter asset references"), cDoNotAlterAssetReferences);
    AssetStoragePtr def = framework->Asset()->GetDefaultAssetStorage();
    std::vector<AssetStoragePtr> storages = framework->Asset()->GetAssetStorages();
    for(size_t i = 0; i < storages.size(); ++i)
    {
        bool readOnly = !storages[i]->Writable();
        storageComboBox->addItem(storages[i]->ToString() + (readOnly ? " (read-only)" : ""), storages[i]->Name());
        if (readOnly) // Mark read-only storages as gray.
        {
            ///\todo Make the font gray and ideally disable the possibility to select the item altogether.
            QFont font = QApplication::font();
            font.setItalic(true);
            storageComboBox->setItemData(i+2, font, Qt::FontRole); // i+2 as we already have the DefaultStorage and DoNotAlterAssetReferences
        }
        if (def && storages[i] == def) // Bold default storage.
        {
            QFont font = QApplication::font();
            font.setBold(true);
            storageComboBox->setItemData(i+2, font, Qt::FontRole); // i+2 as we already have the DefaultStorage and DoNotAlterAssetReferences
            storageComboBox->setCurrentIndex(i+2);
        }
    }
}

void AddContentWindow::RewriteDestinationNames()
{
    QString storageName = CurrentStorageName();
    AssetStoragePtr dest;
    bool doNotAlter = storageName == cDoNotAlterAssetReferences;
    bool useDefault = storageName == cDefaultStorage;
    if (!doNotAlter)
    {
        dest = useDefault ? framework->Asset()->GetDefaultAssetStorage() : framework->Asset()->GetAssetStorageByName(storageName);
        if (!dest)
        {
            LogError("AddContentWindow::RewriteDestinationNames: Could not retrieve asset storage " + storageName + ".");
            GenerateStorageComboBoxContents(); // Regenerate storage combo box items to make sure that we're up-to-date.
            return;
        }
    }

    if (dest && !dest->Writable())
    {
        QString errorMsg = tr("Read-only storage %1 cannot be used for asset upload.").arg(dest->Name());
        uploadStatusLabel->setText(errorMsg);
        uploadStatusLabel->show();
        LogError("AddContentWindow::RewriteDestinationNames: " + errorMsg);
        return;
    }

    uploadStatusLabel->setText("");
    uploadStatusLabel->hide();

    assetTreeWidget->setSortingEnabled(false);

    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*it);
        if (aitem /*&& !aitem->isDisabled()*/)
        {
            if (useDefault)
                aitem->desc.destinationName = AssetAPI::ExtractFilenameFromAssetRef(aitem->text(cColumnAssetDestName).trimmed());
            else if (dest)
                aitem->desc.destinationName = dest->GetFullAssetURL(aitem->text(cColumnAssetDestName).trimmed());
            aitem->setText(cColumnAssetDestName, aitem->desc.destinationName);
            aitem->setDisabled(doNotAlter ? true : false);
            if (aitem->isDisabled() && aitem->isSelected()) // Deselect possible disabled items
                aitem->setSelected(false);
        }
        ++it;
    }

    assetTreeWidget->setSortingEnabled(true);
}

void AddContentWindow::HandleUploadCompleted(IAssetUploadTransfer *transfer)
{
    successfulUploads++;
    uploadStatusLabel->setText("Uploaded " + transfer->AssetRef());
    uploadProgressBar->setValue(uploadProgressBar->value() + progressStep_);
    UpdateUploadStatus(true, transfer->AssetRef());
    CheckUploadTotals();
}

void AddContentWindow::HandleUploadFailed(IAssetUploadTransfer *transfer)
{
    failedUploads_++;
    uploadStatusLabel->setText("Upload failed for " + transfer->AssetRef());
    uploadProgressBar->setValue(uploadProgressBar->value() + progressStep_);
    UpdateUploadStatus(false, transfer->AssetRef());
    CheckUploadTotals();
}

void AddContentWindow::UpdateUploadStatus(bool successful, const QString &assetRef)
{
    QColor statusColor;
    if (successful)
        statusColor = QColor(0, 255, 0, 75);
    else
        statusColor = QColor(255, 0, 0, 75);

    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        AssetWidgetItem *aitem = dynamic_cast<AssetWidgetItem *>(*it);
        assert(aitem);
        if (aitem && !aitem->isDisabled())
        {
            if (aitem->text(cColumnAssetDestName) == assetRef)
            {
                aitem->setBackgroundColor(cColumnAssetTypeName, statusColor);
                aitem->setBackgroundColor(cColumnAssetSourceName, statusColor);
                aitem->setBackgroundColor(cColumnAssetSubname, statusColor);
                aitem->setBackgroundColor(cColumnAssetDestName, statusColor);
                break;
            }
        }
        ++it;
    }
}

void AddContentWindow::CheckUploadTotals()
{
    if (successfulUploads + failedUploads_ == totalUploads_)
    {
        uploadStatusLabel->setText(QString(tr("%1/%2 uploads completed successfully")).arg(successfulUploads).arg(totalUploads_));
        AddEntities();
    }
}
