/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   OgreMaterialEditor.h
 *  @brief  Non-text high-level editing tool for OGRE material scripts.
 */

#pragma once

#include "OgreAssetEditorModuleApi.h"
#include "AssetFwd.h"

#include <QWidget>
#include <QItemDelegate>

class QTabWidget;

class PropertyTableWidget;

/// Non-text high-level editing tool for OGRE material scripts.
class ASSET_EDITOR_MODULE_API OgreMaterialEditor : public QWidget
{
    Q_OBJECT

public:
    OgreMaterialEditor(const AssetPtr &materialAsset, Framework *fw, QWidget *parent = 0);
    ~OgreMaterialEditor();

    void SetMaterialAsset(const AssetPtr &scriptAsset);
    void Populate();

private:
    Q_DISABLE_COPY(OgreMaterialEditor);
    QTabWidget *tabWidget;
    QTabWidget *tuTabWidget;
    Framework *framework;
    AssetWeakPtr asset;
    PropertyTableWidget *shaderAttributeTable;

private slots:
    // Pass attribute setters
    void OpenColorPicker();
    void SetColor(const QColor &color);
    void SetSrcBlendFactor(int);
    void SetDstBlendFactor(int);
    void SetDepthTest(int);
    void SetDepthWrite(int);
    void SetDepthBias(double);
    void SetCullingMode(int);
    void SetLighting(int);
    void SetShadingMode(int);
    void SetFillMode(int);
    void SetColorWrite(int);
    void SetShader(const QString &);
    // TU attribute setters
    void SetTexAssetRef();
    void SetTexCoordSet(int);
    void SetAddrMode(int);
    void SetScrollAnimU(double);
    void SetScrollAnimV(double);
    void SetRotateAnim(double);

    void PopulateShaderAttributes();
    void PopulateTextureUnits(int techIndex, int passIndex);

    void OnAssetTransferSucceeded(AssetPtr asset);
    void OnAssetTransferFailed(IAssetTransfer *transfer, QString reason);
};

///@cond PRIVATE
class SpinBoxDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    SpinBoxDelegate(bool floatingPoint, QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
    const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:
    bool floatingPoint;
};
///@endcond
