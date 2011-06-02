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
#include "SceneAPI.h"
#include "Scene.h"
#include "IComponent.h"
#include "Entity.h"
#include "LoggingFunctions.h"
#include <QDomDocument>

#include "MemoryLeakCheck.h"

EcXmlEditorWidget::EcXmlEditorWidget(Framework *framework, QWidget *parent) :
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

    setAttribute(Qt::WA_DeleteOnClose);
}

EcXmlEditorWidget::~EcXmlEditorWidget()
{
}

void EcXmlEditorWidget::SetEntity(const QList<EntityPtr> &entities)
{
    entities_.clear();
    components_.clear();

    foreach(EntityPtr ent, entities)
        entities_ << ent;

    Refresh();
}

void EcXmlEditorWidget::SetComponent(const QList<ComponentPtr> &components)
{
    entities_.clear();
    components_.clear();

    foreach(ComponentPtr comp, components)
        components_ << comp;

    Refresh();
}

void EcXmlEditorWidget::Refresh()
{
    xmlEdit_->clear();

    ///\todo Check for expired entities & components and drop them.
//    if (entity_.expired())
//        return;
//    EntityPtr entity = entity_.lock();

    QDomDocument temp_doc;

    // Iterate through individually selected components
    if (components_.size() > 0)
    {
        QListIterator<ComponentWeakPtr> it(components_);

        QDomElement entity_elem = temp_doc.createElement("entity");
        temp_doc.appendChild(entity_elem);

        QString id_str;
        id_str.setNum((int)it.peekNext().lock()->ParentEntity()->Id());
        entity_elem.setAttribute("id", id_str);

        while(it.hasNext())
        {
            ComponentPtr component = it.next().lock();
            component->SerializeTo(temp_doc, entity_elem);
        }
    }

    // Insert "entities" tag if we have multiple entities within the same doc, otherwise XML parsing error occurs.
    QDomElement entities_elem = temp_doc.createElement("entities");
    bool multiple = false;
    if (entities_.size() > 1)
    {
        multiple = true;
        temp_doc.appendChild(entities_elem);
        entities_elem.setAttribute("count", QString::number(entities_.size()));
    }

    // Iterate through individually selected entities.
    QListIterator<EntityWeakPtr> it(entities_);
    while(it.hasNext())
    {
        EntityPtr entity = it.next().lock();
        if (!entity)
            continue;

        QDomElement entity_elem = temp_doc.createElement("entity");
        if (multiple)
            entities_elem.appendChild(entity_elem);
        else
            temp_doc.appendChild(entity_elem);
        entity_elem.setAttribute("id", QString::number((int)entity->Id()));

        const Entity::ComponentVector &components = entity->Components();
        for(uint i = 0; i < components.size(); ++i)
        {
            components[i]->SerializeTo(temp_doc, entity_elem);
            if (multiple)
                entities_elem.appendChild(entity_elem);
            else
                temp_doc.appendChild(entity_elem);
        }
    }

    xmlEdit_->setText(temp_doc.toString());
}

void EcXmlEditorWidget::Revert()
{
    ///\todo Check for expired entities & components and drop them.
/*
    foreach(EntityWeakPtr entity, entities_)
        if (entity.expired())
            xmlEdit_->clear();
*/
    Refresh();
}

void EcXmlEditorWidget::Save()
{
    ///\todo Check for expired entities & components and drop them.
/*
    foreach(EntityWeakPtr entity, entities_)
        if (entity.expired())
        {
            xmlEdit_->clear();
            Refresh();
            return;
        }
*/
    QString text = xmlEdit_->toPlainText();
    if (!text.length())
    {
        LogWarning("Empty XML data");
        return;
    }

    QString errorMsg;
    QDomDocument edited_doc;
    if (edited_doc.setContent(text, false, &errorMsg))
    {
        ScenePtr scene = framework_->Scene()->GetDefaultScene();
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
        while(!entity_elem.isNull())
        {
            entity_found = true;
            entity_id_t id = (entity_id_t)entity_elem.attribute("id").toInt();
            EntityPtr entity = scene->GetEntity(id);
            if (entity)
            {
                QDomElement comp_elem = entity_elem.firstChildElement("component");
                while(!comp_elem.isNull())
                {
                    ComponentPtr comp = entity->GetComponent(comp_elem.attribute("type"), comp_elem.attribute("name"));
                    if (comp)
                    {
                        comp->DeserializeFrom(comp_elem, AttributeChange::Default);
                        //comp->ComponentChanged(AttributeChange::Default);
                    }
                    comp_elem = comp_elem.nextSiblingElement("component");
                }
            }
            else
            {
                LogWarning("Could not find entity " + ToString<int>(id) + " in scene!");
            }

            entity_elem = entity_elem.nextSiblingElement("entity");
        }

        // Refresh immediately after, so that any extra stuff is stripped out, and illegal parameters are (hopefully) straightened
        if (entity_found)
            Refresh();
        else
            LogWarning("No entity elements in XML data");
    }
    else
        LogWarning("Could not parse XML data: " + errorMsg.toStdString());
}

void EcXmlEditorWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
        setWindowTitle(tr("EC Attribute XML Editor"));
    else
       QWidget::changeEvent(event);
}
