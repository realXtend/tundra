#ifndef MumbleVoipModule_LinkPlugin_h
#define MumbleVoipModule_LinkPlugin_h

#include <QObject>
#include "Core.h"

namespace MumbleVoip
{

struct LinkedMem;

/**
 *  Implements Mumble Link plugin.
 *  @see http://mumble.sourceforge.net/Link
 *
 */
class LinkPlugin : public QObject
{
	Q_OBJECT
public:
	LinkPlugin();
	virtual ~LinkPlugin();

public slots:

	//! Send data to Mumble client application
	void SendData();

	//! Set name for avatar
	virtual void SetAvatarName(const QString& name);

	//! Set identity for avatar
	virtual void SetAvatarIdentity(const QString& identity);

    //! Set context aka 'group identifier'
    virtual void SetGroupId(const QString& id);

    //! Set application description
    virtual void SetDescription(const QString& description);

	//! Set avatar position vectors
    //!
    //! Left handed coordinate system.
	//!  X positive towards "left".
	//!  Y positive towards "up".
	//!  Z positive towards "into screen".
	//!
	//!  1 unit = 1 meter
	virtual void SetAvatarPosition(Vector3df position, Vector3df front, Vector3df top);

	//! Set camera position vectors
    //!
    //! Left handed coordinate system.
	//!  X positive towards "left".
	//!  Y positive towards "up".
	//!  Z positive towards "into screen".
	//!
	//!  1 unit = 1 meter
	virtual void SetCameraPosition(Vector3df position, Vector3df front, Vector3df top);

protected:
	LinkedMem* linked_mem_;
    QString avatar_name_;
    QString avatar_id_;
    QString avatar_group_id_;
    QString description_;
    Vector3df avatar_position_;
    Vector3df avatar_top_;
    Vector3df avatar_front_;
    Vector3df camera_position_;
    Vector3df camera_top_;
    Vector3df camera_front_;
};

} // end of namespace: MumbleVoip

#endif // MumbleVoipModule_LinkPlugin_h
