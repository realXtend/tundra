// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_UiHelper_h
#define incl_WorldBuildingModule_UiHelper_h

#include <EC_OpenSimPrim.h>

#include <QObject>
#include <QString>
#include <QMap>

#include <QtVariantPropertyManager>
#include <QtVariantEditorFactory>
#include <QtTreePropertyBrowser>
#include <QtVariantProperty>

#include "CustomLineEditFactory.h"

namespace WorldBuilding
{
    namespace Helpers
    {
        class UiHelper : public QObject
        {

        Q_OBJECT

        public:
            // Constructor
            UiHelper();

            // Public containers
            QMap<QString, QtProperty*> editor_items;
            QStringList information_items, rex_prim_data_items, object_shape_items;

            // Public pointers
            QtVariantPropertyManager *variant_manager;
            QtStringPropertyManager *string_manager;
            QtTreePropertyBrowser *browser;

        public slots:
            QString CheckUiValue(QString value);
            QString CheckUiValue(unsigned int value);

            QtTreePropertyBrowser *CreatePropertyBrowser(QWidget *parent, QObject *controller, EC_OpenSimPrim *prim);

        private slots:
            void CollapseSubGroups(QtBrowserItem *main_group);

            QtProperty *CreateInformationGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim);
            QtProperty *CreateRexPrimDataGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim);
            QtProperty *CreateShapeGroup(QtVariantPropertyManager *variant_manager, EC_OpenSimPrim *prim);
  
        private:
            QStringList shape_limiter_zero_to_one; 
            QStringList shape_limiter_minushalf_to_plushalf;
            QStringList shape_limiter_minusone_to_plusone;
            QStringList shape_limiter_zero_to_three;
        };
    }
}

#endif