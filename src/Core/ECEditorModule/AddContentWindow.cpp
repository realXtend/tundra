/**
    For conditions of distribution and use, see copyright notice in LICENSE

    @file   AddContentWindow.cpp
    @brief  Window for adding new content and uploading assets. */

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
#include "IAsset.h"
#include "IAssetStorage.h"
#include "IAssetUploadTransfer.h"
#include "SceneDesc.h"
#include "Scene/Scene.h"
#include "SceneImporter.h"
#include "Transform.h"
#include "LoggingFunctions.h"
#include "CoreException.h"
#include "ConfigAPI.h"

#include "MemoryLeakCheck.h"

namespace
{

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
const int cColumnAssetUpload = 0; ///< Upload column index.
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
        if (column == cColumnEntityId)
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

} // ~unnamed namespace

EntityAndAssetTreeWidget::EntityAndAssetTreeWidget(QWidget *parent) :
    QTreeWidget(parent)
{
    setSelectionMode(QAbstractItemView::ExtendedSelection);
}

void EntityAndAssetTreeWidget::keyPressEvent(QKeyEvent *event)
{
    QTreeWidget::keyPressEvent(event);
    switch (event->key())
    {
        case Qt::Key_Space:
            SelectedItemsList selected = selectedItems();
            if (selected.isEmpty())
                return;

            Qt::CheckState checkedState;
            bool isMixedSelection = false;

            if (selected.size() == 1)
            {
                selected.at(0)->setCheckState(0, (Qt::CheckState)(Qt::Checked - selected.at(0)->checkState(0)));
                return;
            }
            else
            {
                checkedState = selected.at(0)->checkState(0);
                for (SelectedItemsList::const_iterator i = selected.constBegin() + 1; i != selected.constEnd(); ++i)
                    if ((*i)->checkState(0) != checkedState)
                    {
                        isMixedSelection = true;
                        break;
                    }

                ToggleCheckedState(isMixedSelection);
            }
    }
}

void EntityAndAssetTreeWidget::ToggleCheckedState(bool checkAllInsteadOfToggle)
{
    SelectedItemsList selected = selectedItems();
    for (SelectedItemsList::const_iterator i = selected.constBegin(); i != selected.constEnd(); ++i)
        (*i)->setCheckState(0, (checkAllInsteadOfToggle ? Qt::Checked : (Qt::CheckState)(Qt::Checked - (*i)->checkState(0))));
}

AddContentWindow::AddContentWindow(const ScenePtr &dest, QWidget *parent) :
    QWidget(parent),
    framework(dest->GetFramework()),
    scene(dest),
    position(float3::zero),
    uploadProgressStep(0),
    numFailedUploads(0),
    numSuccessfulUploads(0),
    numTotalUploads(0)
{
    setWindowModality(Qt::ApplicationModal/*Qt::WindowModal*/);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Entity And Asset Import"));

    QPixmap nullIcon(16,16); // do a null icon to hide the default ugly one
    nullIcon.fill(Qt::transparent);
    setWindowIcon(nullIcon);

    setStyleSheet(
        "QProgressBar {"
            "border: 1px solid grey; border-radius: 0px; text-align: center; background-color: rgb(244, 244, 244);"
        "}"
        "QProgressBar::chunk {"
            "background-color: qlineargradient(spread:pad, x1:0.028, y1:1, x2:0.972, y2:1, stop:0 rgba(194, 194, 194, 100), stop:1 rgba(115, 115, 115, 100));"
        "}"
    );

    QFont titleFont("Arial", 11);
    titleFont.setBold(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    setLayout(layout);

    // Entities ui
    entityView = new QWidget(this);
    QVBoxLayout *entitiesLayout = new QVBoxLayout(this);
    entitiesLayout->setContentsMargins(0, 0, 0, 0);
    entityView->setLayout(entitiesLayout);

    QLabel *entityLabel = new QLabel(tr("Entities"), this);
    entityLabel->setFont(titleFont);
    entityLabel->setStyleSheet("color: rgb(63, 63, 63);");

    entityTreeWidget = new EntityAndAssetTreeWidget(this);
    entityTreeWidget->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
    entityTreeWidget->setColumnCount(3);
    entityTreeWidget->setHeaderLabels(QStringList(QStringList() << tr("Create") << tr("ID") << tr("Name")));
    entityTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

    QPushButton *selectAllEntitiesButton = new QPushButton(tr("Select All"));
    QPushButton *deselectAllEntitiesButton = new QPushButton(tr("Deselect All"));
    QHBoxLayout *entityButtonsLayout = new QHBoxLayout;
    QSpacerItem *entityButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QSpacerItem *middleSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Fixed);

    QVBoxLayout *entitiesProgressLayout = new QVBoxLayout(this);
    entityStatusLabel = new QLabel(this);
    entityProgressBar = new QProgressBar(this);
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
    layout->addWidget(entityView);

    // Assets ui
    assetView = new QWidget(this);
    QVBoxLayout *assetsLayout = new QVBoxLayout(this);
    assetsLayout->setContentsMargins(0, 0, 0, 0);
    assetView->setLayout(assetsLayout);

    QLabel *assetLabel = new QLabel(tr("Assets"), this);
    assetLabel->setFont(titleFont);
    assetLabel->setStyleSheet("color: rgb(63, 63, 63);");

    assetTreeWidget = new EntityAndAssetTreeWidget(this);
    assetTreeWidget->setColumnCount(5);
    QStringList labels;
    labels << tr("Upload") << tr("Type") << tr("Source name") << tr("Source subname") << tr("Destination name");
    assetTreeWidget->setHeaderLabels(labels);

    QPushButton *selectAllAssetsButton = new QPushButton(tr("Select All"), this);
    QPushButton *deselectAllAssetsButton = new QPushButton(tr("Deselect All"), this);
    QHBoxLayout *assetButtonsLayout = new QHBoxLayout(this);
    QSpacerItem *assetButtonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    QLabel *storageLabel = new QLabel(tr("Asset storage:"), this);
    storageComboBox = new QComboBox(this);

    QVBoxLayout *uploadLayout = new QVBoxLayout(this);
    uploadStatusLabel = new QLabel(this);
    uploadProgressBar = new QProgressBar(this);
    uploadProgressBar->setFixedHeight(20);

    GenerateStorageComboBoxContents();

    uploadLayout->addWidget(uploadStatusLabel);
    uploadLayout->addWidget(uploadProgressBar);

    assetsLayout->addWidget(assetLabel);
    assetsLayout->addLayout(uploadLayout);
    assetsLayout->addWidget(assetTreeWidget);

    assetButtonsLayout->addWidget(selectAllAssetsButton);
    assetButtonsLayout->addWidget(deselectAllAssetsButton);
    assetButtonsLayout->addSpacerItem(assetButtonSpacer);
    assetButtonsLayout->addWidget(storageLabel);
    assetButtonsLayout->addWidget(storageComboBox);
    assetsLayout->insertLayout(-1, assetButtonsLayout);
    layout->addWidget(assetView);

    uploadStatusLabel->hide();
    uploadProgressBar->hide();
    entityStatusLabel->hide();
    entityProgressBar->hide();

    // General controls
    addContentButton = new QPushButton(tr("Add content"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);

    QHBoxLayout *buttonsLayout = new QHBoxLayout(this);
    QSpacerItem *buttonSpacer = new QSpacerItem(20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    buttonsLayout->addSpacerItem(buttonSpacer);
    buttonsLayout->addWidget(addContentButton);
    buttonsLayout->addWidget(cancelButton);
    layout->insertLayout(-1, buttonsLayout);

    resize(800, 400);

    connect(addContentButton, SIGNAL(clicked()), SLOT(AddContent()));
    connect(cancelButton, SIGNAL(clicked()), SLOT(close()));
    connect(selectAllEntitiesButton, SIGNAL(clicked()), SLOT(SelectAllEntities()));
    connect(deselectAllEntitiesButton, SIGNAL(clicked()), SLOT(DeselectAllEntities()));
    connect(selectAllAssetsButton, SIGNAL(clicked()), SLOT(SelectAllAssets()));
    connect(deselectAllAssetsButton, SIGNAL(clicked()), SLOT(DeselectAllAssets()));
    connect(assetTreeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), SLOT(CheckIfColumnIsEditable(QTreeWidgetItem *, int)));
    connect(storageComboBox, SIGNAL(currentIndexChanged(int)), SLOT(RewriteDestinationNames()));

    // If we saved recently used storage, and the storage exists in the current combo box, set it as current index.
    ConfigData configData(ConfigAPI::FILE_FRAMEWORK, cAddContentDialogSetting, cRecentStorageSetting, "", "");
    QString storageName = framework->Config()->Get(configData).toString();
    if (!storageName.isEmpty())
        for(int i = 0; i < storageComboBox->count(); ++i)
            if (storageComboBox->itemData(i) == storageName)
            {
                storageComboBox->setCurrentIndex(i);
                break;
            }

    connect(this, SIGNAL(AssetUploadCompleted(const AssetStoragePtr &, int, int)), SLOT(CreateEntities()));
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
    CenterToMainWindow(); ///< @todo Remove
    QWidget::showEvent(e);
}

void AddContentWindow::AddDescription(const SceneDesc &desc)
{
    sceneDescs.clear();
    sceneDescs.append(desc);
    AddEntities(desc.entities);
    AddAssets(desc, desc.assets);
//    resize(1,1);
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
//    resize(1,1);
}

void AddContentWindow::AddAssets(const QStringList &fileNames)
{
    SceneDesc desc;
    foreach(const QString &file, fileNames)
    {
        AssetDesc ad;
        ad.source = file;
        ad.dataInMemory = false;
        QString type = scene.lock().get() ? scene.lock()->GetFramework()->Asset()->GetResourceTypeFromAssetRef(file) : "Binary";
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
}

void AddContentWindow::AddAssets(const SceneDesc &sceneDesc, const SceneDesc::AssetMap &assetDescs)
{
    if (assetDescs.empty())
    {
        SetAssetsVisible(false);
        return;
    }
    assetTreeWidget->setSortingEnabled(false);

    foreach(const AssetDesc &a, assetDescs)
    {
        AssetWidgetItem *aItem = new AssetWidgetItem(a);

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
        if ((a.typeName == "OgreMaterial" && a.data.isEmpty()) || res == AssetAPI::FileQueryLocalFileMissing)
        {
            // File not found, mark the item red and disable it.
            aItem->setBackgroundColor(cColumnAssetSourceName, QColor(255,0,0,200));
            aItem->setTextColor(cColumnAssetSourceName, Qt::white);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);

            //insert on top of the tree
            assetTreeWidget->insertTopLevelItem(0, aItem);
        }
        else if (res == AssetAPI::FileQueryExternalFile)
        {
            // External reference, mark the item gray and disable it.
            aItem->setBackgroundColor(cColumnAssetSourceName, Qt::gray);
            aItem->setTextColor(cColumnAssetSourceName, Qt::white);
            aItem->setCheckState(cColumnAssetUpload, Qt::Unchecked);
            aItem->setText(cColumnAssetDestName, "");
            aItem->setDisabled(true);

            //insert on top of the tree
            assetTreeWidget->insertTopLevelItem(0, aItem);
        }

        assetTreeWidget->addTopLevelItem(aItem);
    }

    RewriteDestinationNames();

    // Enable sorting, resize header sections to contents.
    assetTreeWidget->setSortingEnabled(true);
    assetTreeWidget->header()->resizeSections(QHeaderView::ResizeToContents);

    // Sort asset items initially so that erroneous are first
    assetTreeWidget->sortItems(cColumnAssetUpload, Qt::AscendingOrder);
}

void AddContentWindow::RewriteAssetReferences(SceneDesc &sceneDesc, const AssetStoragePtr &dest, bool useDefaultStorage)
{
/*
    QString path = QFileInfo(sceneDesc.filename).dir().path();
      QList<SceneDesc::AssetMapKey> keysWithSubname;
      foreach(const SceneDesc::AssetMapKey &key, sceneDesc.assets.keys())
          if (!key.second.isEmpty())
              keysWithSubname.append(key);
*/
    QMutableListIterator<EntityDesc> edIt(sceneDesc.entities);
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
                    QStringList newValues;
                    foreach(QString value, adIt.value().value.split(";", QString::SkipEmptyParts))
                    {
                        QString assetBaseName = AssetAPI::ExtractFilenameFromAssetRef(value);
                        newValues << (useDefaultStorage ? assetBaseName : dest->GetFullAssetURL(assetBaseName));
                        /// @todo Old buggy code below (see also beginning of this function), "value == key.first" never true
                        /// Can be removed for good when the new simplified code path is tested properly.
/*
                        QString subname;
                        if (!keysWithSubname.isEmpty())
                        {
                            ///\todo This string manipulation/crafting doesn't work for .zip files, only for materials and COLLADA files
                            int slashIdx = value.lastIndexOf("/");
                            int dotIdx = value.lastIndexOf(".");
                            QString str = value.mid(slashIdx + 1, dotIdx - slashIdx - 1);

                            foreach(const SceneDesc::AssetMapKey &key, keysWithSubname)
                                if (value == key.first && str == key.second) // Note: value == key.first did not match ever for assets with subname
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
*/
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

AssetStoragePtr AddContentWindow::CurrentStorage() const
{
    const QString storageName = CurrentStorageName();
    return storageName == cDefaultStorage ? framework->Asset()->GetDefaultAssetStorage() : framework->Asset()->GetAssetStorageByName(storageName);
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
    CheckForStorageValidity(); // Enable/disable Add content button
}

void AddContentWindow::DeselectAllAssets()
{
    TreeWidgetSetCheckStateForAllItems(assetTreeWidget, cColumnAssetUpload, Qt::Unchecked);
    CheckForStorageValidity();  // Enable/disable Add content button
}

void AddContentWindow::AddContent()
{
    uploadStatusLabel->setText("");
    uploadProgressBar->setValue(0);
    entityProgressBar->setValue(0);

    // Validate storage here, so no need to do it in CreateNewDesctiption and UploadAssets
    if (!CheckForStorageValidity())
        return;

    CreateNewDesctiption();

    if (filteredDesc.assets.size() > 0)
        UploadAssets();

    if (filteredDesc.assets.isEmpty() && !filteredDesc.entities.empty())
        CreateEntities();

    /// @todo Should user be able to perform more actions than just one?
//    addContentButton->setEnabled(false);
//    storageComboBox->setEnabled(false);
}

SceneDesc AddContentWindow::CurrentContent() const
{
    SceneDesc ret;

    QTreeWidgetItemIterator eit(entityTreeWidget);
    while(*eit)
    {
        EntityWidgetItem *eitem = checked_static_cast<EntityWidgetItem *>(*eit);
        if (eitem->checkState(cColumnEntityCreate) == Qt::Checked)
            ret.entities.append(eitem->desc);
        ++eit;
    }

    QTreeWidgetItemIterator ait(assetTreeWidget);
    while(*ait)
    {
        AssetWidgetItem *aitem = checked_static_cast<AssetWidgetItem *>(*ait);
        if (aitem->checkState(cColumnAssetUpload) == Qt::Checked && !aitem->isDisabled())
            ret.assets.insert(qMakePair(aitem->desc.source, aitem->desc.subname), aitem->desc);
        ++ait;
    }

    return ret;
}

void AddContentWindow::CreateNewDesctiption()
{
    // Merge possible multiple scene descs.
    filteredDesc = SceneDesc();
    filteredDesc.filename = sceneDescs.last().filename; ///< @todo For now, we need to know the filename if it's a Ogre .scene.
    filteredDesc.viewEnabled = sceneDescs.last().viewEnabled;
    foreach(const SceneDesc &desc, sceneDescs)
    {
        filteredDesc.entities.append(desc.entities);
        filteredDesc.assets.unite(desc.assets);
    }

    // Filter entities
    QTreeWidgetItemIterator eit(entityTreeWidget);
    while(*eit)
    {
        EntityWidgetItem *eitem = checked_static_cast<EntityWidgetItem *>(*eit);
        if (eitem->checkState(cColumnEntityCreate) == Qt::Unchecked)
        {
            QList<EntityDesc>::const_iterator ei = qFind(filteredDesc.entities, eitem->desc);
            if (ei != filteredDesc.entities.end())
                filteredDesc.entities.removeOne(*ei);
        }

        ++eit;
    }

    // Rewrite components' asset refs, if storage selected.
    bool doNotAlter = CurrentStorageName() == cDoNotAlterAssetReferences;
    bool useDefault = CurrentStorageName() != cDefaultStorage;
    AssetStoragePtr dest = CurrentStorage();
    if (!doNotAlter)
        RewriteAssetReferences(filteredDesc, dest, useDefault);

    // Filter assets
    RefMap refs;
    QTreeWidgetItemIterator ait(assetTreeWidget);
    while(*ait)
    {
        AssetWidgetItem *aitem = checked_static_cast<AssetWidgetItem *>(*ait);
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
            if (aitem->desc.typeName == "Texture")
            {
                int idx = aitem->desc.source.lastIndexOf("/");
                refs[aitem->desc.source.mid(idx != -1 ? idx + 1 : 0).trimmed()] = aitem->desc.destinationName;
            }
        }

        ++ait;
    }

    // Rewrite asset refs
    QMutableMapIterator<SceneDesc::AssetMapKey, AssetDesc> rewriteIt(filteredDesc.assets);
    while(rewriteIt.hasNext())
    {
        rewriteIt.next();
        if (rewriteIt.value().typeName.contains("OgreMaterial", Qt::CaseInsensitive))
            ///\todo This logic will be removed in the future, as we need it generic for any types of assets.
            ReplaceReferences(rewriteIt.value().data, refs);
    }
}

bool AddContentWindow::CheckForStorageValidity()
{
    AssetStoragePtr dest = CurrentStorage();
    QString storageName = CurrentStorageName();
    bool doNotAlter = storageName == cDoNotAlterAssetReferences;
    if (!doNotAlter && !dest)
    {
        QString errorMsg = tr("Could not retrieve asset storage ") + storageName + ".";
        uploadStatusLabel->setText("<b>" + errorMsg + "</b>");
        uploadStatusLabel->show();
        LogDebug("AddContentWindow::CheckForStorageValidity: " + errorMsg);
        GenerateStorageComboBoxContents(); // Regenerate storage combo box items to make sure that we're up-to-date.
        return false;
    }

    // Note: if no assets are going to be uploaded, no need to show the error and cancel the flow
    if (dest && !dest->Writable() && !CurrentContent().assets.empty()) // Can't use filteredDesc.assets here as it's not probably created yet.
    {
        QString errorMsg = tr("Read-only storage %1 cannot be used for asset upload.").arg(dest->Name());
        uploadStatusLabel->setText("<b>" + errorMsg + "</b>");
        uploadStatusLabel->show();
        addContentButton->setEnabled(false);
        LogDebug("AddContentWindow::CheckForStorageValidity: " + errorMsg);
        return false;
    }

    addContentButton->setEnabled(true);
    return true;
}

void AddContentWindow::UploadAssets()
{
    AssetStoragePtr dest = CurrentStorage();
    assert(dest);

    if (filteredDesc.assets.size() > 0)
    {
        entityStatusLabel->setText(tr("Waiting for asset uploads to finish..."));
        LogDebug(QString("Starting uploading of %1 asset%2.").arg(filteredDesc.assets.size()).arg(filteredDesc.assets.size() == 1 ? "." : "s."));
    }
    numTotalUploads = 0;
    uploadProgressStep = 0;
    numFailedUploads = 0;
    numSuccessfulUploads = 0;
    foreach(const AssetDesc &ad, filteredDesc.assets)
    {
        try
        {
            AssetUploadTransferPtr transfer;
            if (ad.dataInMemory)
                transfer = framework->Asset()->UploadAssetFromFileInMemory((const u8*)ad.data.data(), ad.data.size(), dest, ad.destinationName);
            else
            {
                AssetAPI::AssetRefType refType = AssetAPI::ParseAssetRef(ad.source);
                if (refType == AssetAPI::AssetRefLocalPath || refType == AssetAPI::AssetRefRelativePath)
                    transfer = framework->Asset()->UploadAssetFromFile(ad.source, dest, ad.destinationName);
                else
                {
                    AssetPtr asset = framework->Asset()->GetAsset(ad.source);
                    if (asset)
                    {
                        if (!asset->DiskSource().isEmpty() && QFile::exists(asset->DiskSource()))
                            transfer = framework->Asset()->UploadAssetFromFile(asset->DiskSource(), dest, ad.destinationName);
                        else
                        {
                            QByteArray data = asset->GetRawData();
                            if (data.length() > 0)
                                transfer = framework->Asset()->UploadAssetFromFileInMemory(data, dest->Name(), ad.destinationName);
                            else
                                LogError("Cannot upload asset '" + ad.source + "' from memory to destination '" + ad.destinationName + "'! The source asset serialized to 0 size!");
                        }
                    }
                    else
                        LogError("Cannot upload asset from source '" + ad.source + "' to destination '" + ad.destinationName + "'! The source asset location is unknown!");
                }
            }

            if (transfer &&
                connect(transfer.get(), SIGNAL(Completed(IAssetUploadTransfer *)),
                    SLOT(HandleUploadCompleted(IAssetUploadTransfer *)), Qt::UniqueConnection) &&
                connect(transfer.get(), SIGNAL(Failed(IAssetUploadTransfer *)),
                    SLOT(HandleUploadFailed(IAssetUploadTransfer *)), Qt::UniqueConnection))
            {
                numTotalUploads++;
            }
        }
        catch(const Exception &e)
        {
            LogError(std::string(e.what()));
        }
    }

    uploadProgressStep = numTotalUploads > 0 ? 100 / numTotalUploads : 100;

    if (numTotalUploads > 0)
    {
        uploadStatusLabel->show();
        uploadProgressBar->show();
    }
    else
    {
        /// @todo Hide this too?
        //uploadStatusLabel->hide();
        uploadProgressBar->hide();
    }

//    QMessageBox::critical(this, tr("Uploading"), tr("Starting uploads failed"));
}

bool AddContentWindow::CreateEntities()
{
//    entityStatusLabel->setText("");
//    entityStatusLabel->hide();

    entityStatusLabel->show();
    entityProgressBar->show();

    ScenePtr destScene = scene.lock();
    if (!destScene)
    {
        QString msg = tr("No destination scene. Can not create entities!");
        LogError("AddContentWindow::CreateEntities: " + msg);
        entityStatusLabel->setText("<b>" + msg + "</b>");
        return false;
    }

    if (!filteredDesc.entities.empty())
    {
        QList<Entity *> entities;
        entities = destScene->CreateContentFromSceneDesc(filteredDesc, false, AttributeChange::Default);

        if (!entities.empty())
        {
            QString msg = QString(tr("%1/%2 entities created successfully")).arg(entities.count()).arg(filteredDesc.entities.count());
            entityStatusLabel->setText(msg);
            if (position != float3::zero)
                SceneStructureModule::CentralizeEntitiesTo(position, entities);
            emit EntitiesCreated(entities);
            /// @todo Remove emitting of Completed
            AssetStoragePtr storage = CurrentStorage();
            emit Completed(entityProgressBar->value() > 0, storage ? storage->BaseURL() : "");
        }
        else
        {
            QString msg = tr("No entities were created, even if the intent was to create %1.").arg(filteredDesc.entities.count());
            entityStatusLabel->setText("<b>" + msg + "</b>");
            return false;
        }

        entityProgressBar->setValue(100);
    }
    else
    {
        QString msg = tr("No entities were selected for creation.");
        entityStatusLabel->setText(msg);
    }

    /// @todo The following is not ideal here, but in AddContent().
    cancelButton->setText(tr("Close"));
    addContentButton->setEnabled(true);
    storageComboBox->setEnabled(true);

//    if (failed)
//        QMessageBox::warning(this, tr("Entity Creation"), tr("Entity creation failed."));

    return true;
}

void AddContentWindow::CenterToMainWindow()
{
    if (framework->Ui()->MainWindow())
    {
        QRect mainRect = framework->Ui()->MainWindow()->rect();
        QPoint mainCenter = framework->Ui()->MainWindow()->pos() + mainRect.center();
        move(mainCenter.x()-width()/2, (mainCenter.y()-height()/2) >= 0 ? mainCenter.y()-height()/2 : 0);
    }
}

void AddContentWindow::SetEntitiesVisible(bool visible)
{
    entityView->setVisible(visible);
}

void AddContentWindow::SetAssetsVisible(bool visible)
{
    assetView->setVisible(visible);
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
    uploadStatusLabel->setText("");
    uploadStatusLabel->hide();

    bool doNotAlter = CurrentStorageName() == cDoNotAlterAssetReferences;
    bool useDefault = CurrentStorageName() == cDefaultStorage;
    AssetStoragePtr dest = CurrentStorage(); // dest might be null, but we have null check for it below

    assetTreeWidget->setSortingEnabled(false);

    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        AssetWidgetItem *aitem = checked_static_cast<AssetWidgetItem *>(*it);
        if (useDefault)
            aitem->desc.destinationName = AssetAPI::ExtractFilenameFromAssetRef(aitem->text(cColumnAssetDestName).trimmed());
        else if (dest)
            aitem->desc.destinationName = dest->GetFullAssetURL(aitem->text(cColumnAssetDestName).trimmed());
        aitem->setText(cColumnAssetDestName, aitem->desc.destinationName);
        aitem->setDisabled(doNotAlter);
        if (aitem->isDisabled() && aitem->isSelected()) // Deselect possible disabled items
            aitem->setSelected(false);

        ++it;
    }

    assetTreeWidget->setSortingEnabled(true);

    CheckForStorageValidity();
}

void AddContentWindow::HandleUploadCompleted(IAssetUploadTransfer *transfer)
{
    HandleUploadProgress(true, transfer);
}

void AddContentWindow::HandleUploadFailed(IAssetUploadTransfer *transfer)
{
    HandleUploadProgress(false, transfer);
}

void AddContentWindow::HandleUploadProgress(bool successful, IAssetUploadTransfer *transfer)
{
    if (successful)
        ++numSuccessfulUploads;
    else
        ++numFailedUploads;

    uploadStatusLabel->setText(successful ? "Uploaded " : "Upload failed for " + transfer->AssetRef());
    uploadProgressBar->setValue(uploadProgressBar->value() + uploadProgressStep);

    const QColor statusColor = successful ? QColor(0, 255, 0, 75) :  QColor(255, 0, 0, 75);

    QTreeWidgetItemIterator it(assetTreeWidget);
    while(*it)
    {
        AssetWidgetItem *aitem = checked_static_cast<AssetWidgetItem *>(*it);
        if (!aitem->isDisabled() && aitem->text(cColumnAssetDestName) == transfer->AssetRef())
        {
            aitem->setBackgroundColor(cColumnAssetTypeName, statusColor);
            aitem->setBackgroundColor(cColumnAssetSourceName, statusColor);
            aitem->setBackgroundColor(cColumnAssetSubname, statusColor);
            aitem->setBackgroundColor(cColumnAssetDestName, statusColor);
            break;
        }
        ++it;
    }

    if (numSuccessfulUploads + numFailedUploads == numTotalUploads)
    {
        uploadStatusLabel->setText(QString(tr("%1/%2 uploads completed successfully")).arg(numSuccessfulUploads).arg(numTotalUploads));
        // Save most recently used storage to config.
        ConfigData c(ConfigAPI::FILE_FRAMEWORK, cAddContentDialogSetting, cRecentStorageSetting, CurrentStorageName());
        framework->Config()->Set(c);
        // Emit AssetUploadCompleted which initiates CreateEntities
        emit AssetUploadCompleted(CurrentStorage(), numSuccessfulUploads, numFailedUploads);
    }
}
