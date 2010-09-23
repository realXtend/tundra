// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_EC_3DCanvas_EC_3DCanvas_h
#define incl_EC_3DCanvas_EC_3DCanvas_h

#include "IComponent.h"
#include "Declare_EC.h"

#include <QMap>

namespace Scene
{
    class Entity;
}

namespace Foundation
{
    class Framework;
}

namespace Ogre
{
    class TextureManager;
    class MaterialManager;
}


class QWidget;
class QTimer;

/**

<table class="header">
<tr>
<td>
<h2>3DCanvas</h2>
Makes the entity a light source.

Registered by RexLogic::RexLogicModule.

<b>No Attributes</b>


<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Start": 
<li>"Setup":
<li>"SetWidget":
<li>"SetRefreshRate":
<li>"SetSubmesh":
<li>"SetSubmeshes":
</ul>

<b>Reacts on the following actions:</b>
<ul>
<li>..
</ul>
</td>
</tr>

Does not emit any actions.

<b>Depends on the component OgreCustomObject and OgreMesh</b>. 
</table>

*/

class EC_3DCanvas : public IComponent
{
    Q_OBJECT
    DECLARE_EC(EC_3DCanvas);

public:
    ~EC_3DCanvas();

public slots:
    void Start();
    void Setup(QWidget *widget, const QList<uint> &submeshes, int refresh_per_second);

    void SetWidget(QWidget *widget);
    void SetRefreshRate(int refresh_per_second);
    void SetSubmesh(uint submesh);
    void SetSubmeshes(const QList<uint> &submeshes);

    QWidget *GetWidget() { return widget_; }

private:
    explicit EC_3DCanvas(IModule *module);
    void UpdateSubmeshes();

private slots:
    void WidgetDestroyed(QObject *obj);
    void Update();

private:
    QWidget *widget_;
    QList<uint> submeshes_;
    QTimer *refresh_timer_;

    QMap<uint, std::string> restore_materials_;
    std::string material_name_;
    std::string texture_name_;

    int update_interval_msec_;
    bool update_internals_;
};

#endif
