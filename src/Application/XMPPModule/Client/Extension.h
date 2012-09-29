/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   Extension.h
 *  @brief  Abstract class for XMPP:Client extensions.
 */

#ifndef incl_XMPP_Extension_h
#define incl_XMPP_Extension_h

#include "qxmpp/QXmppClient.h"

#include <QObject>
#include <QString>

typedef double f64;

namespace Foundation
{
    class Framework;
}

namespace XMPP
{
class Client;

//! Base class for any XMPP::Client extensions.
class Extension : public QObject
{
    Q_OBJECT

public:
    virtual ~Extension() {}
    explicit Extension(const QString &Name);

    //! Needs to be implemented in all extension implemenations
    //! Gets called in when Clients addExtension() is performed.
    virtual void Initialize(Client *client) = 0;

    //! Not purely virtual. Implement if you need to sucsribe to
    //! frame ticks.
    virtual void Update(f64 frametime);

public slots:
    QString Name() const;

protected:
    const QString name_;
};

} // end of namespace: XMPP

#endif // incl_XMPP_Extension_h
