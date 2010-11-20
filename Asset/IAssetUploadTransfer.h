// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Asset_IAssetUploadTransfer_h
#define incl_Asset_IAssetUploadTransfer_h

#include <QObject>

class IAssetUploadTransfer : public QObject
{
    Q_OBJECT

public:
    virtual ~IAssetUploadTransfer() {}

    /// Returns the current transfer progress in the range [0, 1].
    float Progress() const { return 0.f; }

signals:
    void Completed(IAssetUploadTransfer *transfer);

    void Failed(IAssetUploadTransfer *transfer);
};

#endif
