/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   EcXmlEditorWidget.cpp
 *  @brief  Entity-component XML editor widget used for editing EC attributes in XML format.
 */

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "EcXmlEditorWidget.h"
#include "ECEditorModule.h"

#include "Framework.h"
#include "SceneManager.h"
#include "ComponentInterface.h"
#include "Entity.h"
#include "XMLUtilities.h"

#include <QDomDocument>

#include "MemoryLeakCheck.h"

namespace ECEditor
{

EcXmlEditorWidget::EcXmlEditorWidget(Foundation::Framework *framework, QWidget *parent) :
    QWidget(parent), framework_(framework), xmlEdit_(0)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setObjectName("mainlayout");
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);
    setWindowTitle(tr("EC Attribute XML Editor"));
    resize(300, 300);

    xmlEdit_ = new QTextEdit(this);
    layout->addWidget(xmlEdit_);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->setObjectName("buttonlayout");

    QSpacerItem *spacer = new QSpacerItem(20, 40, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlayout->addSpacerItem(spacer);

    QPushButton *saveButton = new QPushButton(tr("Save"), this);
    connect(saveButton, SIGNAL(pressed()), this, SLOT(Save()));
    hlayout->addWidget(saveButton);

    QPushButton *revertButton = new QPushButton(tr("Revert"), this);
    connect(revertButton, SIGNAL(pressed()), this, SLOT(Revert()));
    hlayout->addWidget(revertButton);

    layout->addLayout(hlayout);
}

EcXmlEditorWidget::~EcXmlEditorWidget()
{
}

void EcXmlEditorWidget::SetEntity(Scene::EntityPtr entity)
{
    entity_ = entity;
    component_.reset();
    Refresh();
}

void EcXmlEditorWidget::SetComponent(Foundation::ComponentPtr component)
{
    component_ = component;

    Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
    if (scene)
        entity_ = scene->GetEntity(component->GetParentEntity()->GetId());

    Refresh();
}

void EcXmlEditorWidget::Refresh()
{
    xmlEdit_->clear();
    if (entity_.expired())
        return;

    Scene::EntityPtr entity = entity_.lock();

    QDomDocument temp_doc;
    QDomElement entity_elem = temp_doc.createElement("entity");
    temp_doc.appendChild(entity_elem);

    QString id_str;
    id_str.setNum((int)entity->GetId());
    entity_elem.setAttribute("id", id_str);

    if (component_.lock())
    {
        // We're single component
        component_.lock()->SerializeTo(temp_doc, entity_elem);
    }
    else
    {
        // We're editing entity
        const Scene::Entity::ComponentVector &components = entity->GetComponentVector();
        for(uint i = 0; i < components.size(); ++i)
            if (components[i]->IsSerializable())
                components[i]->SerializeTo(temp_doc, entity_elem);

        temp_doc.appendChild(entity_elem);
    }

    xmlEdit_->setText(temp_doc.toString());
}

void EcXmlEditorWidget::Revert()
{
    if (entity_.expired())
    {
        xmlEdit_->clear();
        return;
    }

    Refresh();
}

void EcXmlEditorWidget::Save()
{
    if (entity_.expired())
    {
        xmlEdit_->clear();
        return;
    }

    QString text = xmlEdit_->toPlainText();
    if (!text.length())
    {
        ECEditorModule::LogWarning("Empty XML data");
        return;
    }

    QDomDocument edited_doc;
    if (edited_doc.setContent(text))
    {
        Scene::ScenePtr scene = framework_->GetDefaultWorldScene();
        if (!scene)
            return;

        // Check if multi-entity or single-entity
        QDomElement entities_elem = edited_doc.firstChildElement("entities");

        // Deserialize all entities/components contained in the data, provided we still find them from the scene
        QDomElement entity_elem;
        if (!entities_elem.isNull())
            entity_elem = entities_elem.firstChildElement("entity");
        else
            entity_elem = edited_doc.firstChildElement("entity");

        bool entity_found = false;
        while (!entity_elem.isNull())
        {
            entity_found = true;
            entity_id_t id = (entity_id_t)RexTypes::ParseInt(entity_elem.attribute("id").toStdString());
            Scene::EntityPtr entity = scene->GetEntity(id);
            if (entity)
            {
                QDomElement comp_elem = entity_elem.firstChildElement("component");
                while (!comp_elem.isNull())
                {
                    Foundation::ComponentInterfacePtr comp = entity->GetComponent(comp_elem.attribute("type").toStdString());
                    if (comp)
                    {
                        comp->DeserializeFrom(comp_elem, Foundation::Local);
                        // Trigger sync to network
                        comp->ComponentChanged(Foundation::Local);
                    }
                    comp_elem = comp_elem.nextSiblingElement("component");
                }
            }
            else
            {
                ECEditorModule::LogWarning("Could not find entity " + ToString<int>(id) + " in scene!");
            }

            entity_elem = entity_elem.nextSiblingElement("entity");
        }

        // Refresh immediately after, so that any extra stuff is stripped out, and illegal parameters are (hopefully) straightened
        if (entity_found)
            Refresh();
        else
            ECEditorModule::LogWarning("No entity elements in XML data");
    }
    else
        ECEditorModule::LogWarning("Could not parse XML data");
}

void EcXmlEditorWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        graphicsProxyWidget()->setWindowTitle(tr("EC Attribute XML Editor"));
    else
       QWidget::changeEvent(event);
}

}
