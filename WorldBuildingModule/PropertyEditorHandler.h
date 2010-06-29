// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_PropertyEditorHandler_h
#define incl_WorldBuildingModule_PropertyEditorHandler_h

#include <QObject>
#include <QtProperty>
#include <QVariant>

#include "UiHelper.h"

class QtProperty;
class QtAbstractPropertyBrowser;
class QtTreePropertyBrowser;

class EC_OpenSimPrim;

namespace WorldBuilding
{
    class PropertyEditorHandler : public QObject
    {
        
    Q_OBJECT

    public:
        PropertyEditorHandler(Helpers::UiHelper *ui_helper, QObject *parent = 0);
        virtual ~PropertyEditorHandler();

    public slots:
        void PrimSelected(EC_OpenSimPrim *prim);
        void SetCurrentPrim(EC_OpenSimPrim *prim);
        void ClearCurrentPrim();
        bool HasCurrentPrim();
        
        void PrimValueChanged(QtProperty *prop, const QString &value);
        void PrimValueChanged(QtProperty *prop, const QVariant &value);

        void CreatePropertyBrowser(QWidget *parent, QLayout *container, EC_OpenSimPrim *prim);

    private slots:
        bool IsTitleProperty(QtProperty *prop);
        //void SetCorrectedValue(QtProperty *prop, const QVariant &corrected_value);

    private:
        EC_OpenSimPrim *current_prim_;
        Helpers::UiHelper *ui_helper_;

        QtAbstractPropertyBrowser *property_browser_;
    };
}

#endif