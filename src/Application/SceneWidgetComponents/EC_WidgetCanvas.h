// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_SceneWidgetComponents_EC_WidgetCanvas_h
#define incl_SceneWidgetComponents_EC_WidgetCanvas_h

#include "SceneWidgetComponentsApi.h"
#include "IComponent.h"
#include "IAttribute.h"
#include "OgreModuleFwd.h"

#include <QMap>
#include <QImage>
#include <QPointer>
#include <QWidget>
#include <QString>

namespace Ogre
{
    class TextureManager;
    class MaterialManager;
}

class QTimer;

/**

<table class="header">
<tr>
<td>
<h2>WidgetCanvas</h2>

\note This component will not sync to network, it is made for local rendering use. It is also not meant to be used directly from the entity-component editor, as you need to pass QWidget* etc for it to do anything. Other components utilizes it when they need QWidget painting done to a 3D object. 

Paints UI widgets on to a 3D object surface via EC_Mesh and a submesh index. So a EC_Mesh needs to be present on the entity this component is used.

Registered by SceneWidgetComponents plugin.

<b>No Attributes</b>

<b>Exposes the following scriptable functions:</b>
<ul>
<li>"Start":
<li>"Update":
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

class SCENEWIDGET_MODULE_API EC_WidgetCanvas : public IComponent
{
    Q_OBJECT
    COMPONENT_NAME("EC_WidgetCanvas", 35)

public:
    explicit EC_WidgetCanvas(Scene *scene);
    ~EC_WidgetCanvas();

public slots:
    void Start();
    void Stop();
    void Update(QImage &buffer);
    void Update();
    void Setup(QWidget *widget, const QList<uint> &submeshes, int refresh_per_second);
    void RestoreOriginalMeshMaterials();

    void SetWidget(QWidget *widget);
    void SetRefreshRate(int refresh_per_second);
    void SetSubmesh(uint submesh);
    void SetSubmeshes(const QList<uint> &submeshes);
    void SetSelfIllumination(bool illuminating);

    QWidget *GetWidget() { return widget_; }
    const int GetRefreshRate() { return update_interval_msec_; }
    QList<uint> GetSubMeshes() { return submeshes_; }

    QString GetMaterialName() { return QString::fromStdString(material_name_); }

private:
    void UpdateSubmeshes();

private slots:
    void WidgetDestroyed(QObject *obj);
    void MeshMaterialsUpdated(uint index, const QString &material_name);

    /// Monitors when parent entity is set.
    void ParentEntitySet();

    /// Monitors this entitys removed components.
    void ComponentRemoved(IComponent *component, AttributeChange::Type change);

private:
    QPointer<QWidget> widget_;
    QList<uint> submeshes_;
    QTimer *refresh_timer_;

    QMap<uint, std::string> restore_materials_;
    std::string material_name_;
    std::string texture_name_;

    int update_interval_msec_;
    bool update_internals_;

    QImage buffer_;
    bool mesh_hooked_;
};

#endif
