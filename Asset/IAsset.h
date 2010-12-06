// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAsset_h
#define incl_Asset_IAsset_h

#include <QObject>
#include <vector>

#include "CoreTypes.h"
#include "AssetFwd.h"
#include "AssetReference.h"

class IAsset : public QObject
{
    Q_OBJECT

public:
    IAsset(const QString &type_, const QString &name_)//, const QString &ref_)
    :type(type_), name(name_)//, ref(ref_)
    {
    }

    virtual ~IAsset() {}

public slots:

    /// Returns the type of this asset. The type of an asset cannot change during the lifetime of the instance of an asset.
    QString Type() const { return type; }

    /// Returns the unique name of this asset. The name of an asset cannot change during the lifetime of the instance of an asset.
    QString Name() const { return name; }
/*
    /// Returns the AssetRef address that can be used to point to this asset in an asset storage.
    QString Ref() const { return ref; }

    /// Specifies the source where this asset is loaded from.
    void SetRef(const QString &newRef) { ref = newRef; }
*/
public:
    /// Loads this asset from file.
//    virtual void LoadFromFile(QString filename);

    /// Loads this asset from the specified file data in memory. Loading an asset from memory cannot change its name or type.
    /// Returns true if loading succeeded, false otherwise.
    virtual bool LoadFromFileInMemory(const u8 *data, size_t numBytes) = 0;

    /// Stores this asset to disk to the given file.
 //   virtual void SaveToFile(const QString &filename);

    /// Goes through the contents of this asset and computes a hash that identifies the data.
//    Hash ComputeContentHash() const;

    /// Called whenever another asset this asset depends on is loaded.
    virtual void DependencyLoaded(AssetPtr dependee) { }

    /// Returns all the assets this asset refers to (but not the references those assets refer to).
    virtual std::vector<AssetReference> FindReferences() const = 0;

    /// Returns true if the replace succeeds.
//    bool ReplaceReference(const QString &oldRef, const QString &newRef);

    /// Returns all the assets this asset refers to, and the assets those assets refer to, and so on.
    std::vector<AssetReference> FindReferencesRecursive() const;

//   virtual void LoadGPU();
//   void UnloadGPU();

//    void UnloadRawData() { rawAssetData.clear(); }

//    void ReloadRawData() { // load from cache; }

    /// Points to the actual asset if it has been loaded in. This member is implemented for legacy purposes to help 
    /// transition period to new Asset API. Will be removed. -jj
    Foundation::AssetInterfacePtr assetPtr;
    Foundation::ResourcePtr resourcePtr;

    QString ToString() const { return (Name().isEmpty() ? "(noname)" : Name()) + " (" + (Type().isEmpty() ? "notype" : Type()) + ")"; }
private:
    /// Specifies the provider this asset was downloaded from.
    AssetProviderWeakPtr provider;

    QString type;
    QString name;
//    QString ref;
};

#endif
