// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "PropertyEditorHandler.h"
#include "WorldBuildingModule.h"

#include <EC_OpenSimPrim.h>

#include <QtAbstractPropertyBrowser>
#include <QtTreePropertyBrowser>

namespace WorldBuilding
{
    PropertyEditorHandler::PropertyEditorHandler(Helpers::UiHelper *ui_helper, QObject *parent) :
        QObject(parent),
        ui_helper_(ui_helper),
        current_prim_(0),
        property_browser_(0)
    {
    }

    PropertyEditorHandler::~PropertyEditorHandler()
    {
    }

    void PropertyEditorHandler::SetEditorVisible(bool visible)
    {
        if (property_browser_)
            property_browser_->setVisible(visible);
    }

    void PropertyEditorHandler::SetCurrentPrim(EC_OpenSimPrim *prim)
    {
        current_prim_ = prim;
    }

    void PropertyEditorHandler::ClearCurrentPrim()
    {
        current_prim_ = 0;
    }

    bool PropertyEditorHandler::HasCurrentPrim()
    {
        if (current_prim_)
            return true;
        else
            return false;
    }

    void PropertyEditorHandler::PrimSelected(EC_OpenSimPrim *prim)
    {
        ClearCurrentPrim();

        // Update property editor ui with selected prim property values
        // Clear the current prim so no networking will be done in setter listeners
        foreach(QString property_name, ui_helper_->editor_items.keys())
        {
            QtProperty *prop = ui_helper_->editor_items[property_name];
            QVariant prop_value = prim->property(property_name.toStdString().c_str());
            if (prop_value.type() == QVariant::String)
                ui_helper_->string_manager->setValue(prop, prop_value.toString());
            else
                ui_helper_->variant_manager->setValue(prop, prop_value);
        }

        SetCurrentPrim(prim);
    }

    void PropertyEditorHandler::PrimValueChanged(QtProperty *prop, const QString &value)
    {
        if (!current_prim_)
            return;
        if (IsTitleProperty(prop))
            return;
        PrimValueChanged(prop, QVariant(value));
    }

    void PropertyEditorHandler::PrimValueChanged(QtProperty *prop, const QVariant &value)
    {
        if (!current_prim_)
            return;

        std::string prop_std = prop->propertyName().toStdString();
        if (current_prim_->setProperty(prop_std.c_str(), value))
        {
            if (ui_helper_->rex_prim_data_items.contains(prop->propertyName()))
                current_prim_->SendRexPrimDataUpdate();
            else if (ui_helper_->object_shape_items.contains(prop->propertyName()))
                current_prim_->SendObjectShapeUpdate();
            else if (prop->propertyName() == "Name")
                current_prim_->SendObjectNameUpdate();
            else if (prop->propertyName() == "Description")
                current_prim_->SendObjectDescriptionUpdate();
            else
                WorldBuildingModule::LogDebug(QString("The property %1 is not yet sent to the server by Naali").arg(prop->propertyName()).toStdString());
        }
    }

    bool PropertyEditorHandler::IsTitleProperty(QtProperty *prop)
    {
        if (prop->subProperties().count() <= 0)
            return false;

        if (!prop->whatsThis().endsWith(" items)"))
            return false;

        EC_OpenSimPrim *saved_prim = current_prim_;
        ClearCurrentPrim();

        ui_helper_->string_manager->setValue(prop, prop->whatsThis());
        SetCurrentPrim(saved_prim);

        return true;
    }

    // Not used as limits are set into the editor itself, might need this later for path revolutions

    //void PropertyEditorHandler::SetCorrectedValue(QtProperty *prop, const QVariant &corrected_value)
    //{
    //    // Avoid us getting again to the function that called us
    //    EC_OpenSimPrim *saved_prim = current_prim_;
    //    current_prim_ = 0;
    //    // Set the corrected value with the appropriate manager
    //    bool added_debug = false;
    //    if (corrected_value.type() == QVariant::String)
    //    {
    //        ui_helper_->string_manager->setValue(prop, corrected_value.toString());
    //        added_debug = true;
    //    }
    //    else
    //    {
    //        ui_helper_->variant_manager->setValue(prop, corrected_value);
    //        added_debug = true;
    //    }
    //    
    //    if (!added_debug)
    //        qDebug() << "Failed to find a manager for property " << prop << " this should never happen!";
    //    else
    //        qDebug() << "Corrected value to UI, due to EC_OpenSimPrim setter correction!";
    //    current_prim_ = saved_prim;
    //}

    void PropertyEditorHandler::CreatePropertyBrowser(QWidget *parent, QLayout *container, EC_OpenSimPrim *prim)
    {
        if (property_browser_)
        {
            PrimSelected(prim);
            return;
        }

        property_browser_ = ui_helper_->CreatePropertyBrowser(parent, this, prim);
        container->addWidget(property_browser_);

        connect(ui_helper_->variant_manager, SIGNAL(valueChanged(QtProperty*, const QVariant&)), 
                SLOT(PrimValueChanged(QtProperty*, const QVariant&)));
        connect(ui_helper_->string_manager, SIGNAL(valueChanged(QtProperty*, const QString&)), 
                SLOT(PrimValueChanged(QtProperty*, const QString&)));

        SetCurrentPrim(prim);
    }
}