// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_CameraInputModule_CameraInput_h
#define incl_CameraInputModule_CameraInput_h

#include <QObject>
#include <QImage>

class CameraInputModule;
namespace Foundation { class Framework; }

class CameraInput : public QObject
{

Q_OBJECT

public:
    friend class CameraInputModule;

    /// Default constructor.
    CameraInput(QObject *parent, Foundation::Framework *framework);

    /// Destructor.
    virtual ~CameraInput();

public slots:
    /// Returns if we have a active device.
    /// \return bool True if we have a device and frameUpdate is being emitted, otherwise false.
    bool HasDevice();

    /// Return the current captured frame.
    /// \return QImage The current captured frame.
    const QImage &CurrentFrame();

signals:
    /// This signal is emitted when the current captured frame is updated.
    void frameUpdate(const QImage &frame);

protected:
    /// Protected function that gets called by the friend class CameraInputModule.
    void SetEnabled(bool enabled);
    
    /// Protected function that gets called by the friend class CameraInputModule.
    void SetFrame(const QImage &frame);

private:
    Foundation::Framework *framework_;
    bool hasDevice_;
    QImage currentFrame_;
};

#endif