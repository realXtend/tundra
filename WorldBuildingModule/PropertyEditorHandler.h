//$ HEADER_MOD_FILE $
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
        void SetEditorVisible(bool visible);
        void PrimSelected(EC_OpenSimPrim *prim);
        void SetCurrentPrim(EC_OpenSimPrim *prim);
        void ClearCurrentPrim();
        bool HasCurrentPrim();
        
        void PrimValueChanged(QtProperty *prop, const QString &value);
        void PrimValueChanged(QtProperty *prop, const QVariant &value);
        void CreatePropertyBrowser(QWidget *parent, QLayout *container, EC_OpenSimPrim *prim);
//$ BEGIN_MOD $
		/*! Creates the property widget
		 *	\return property widget
		 */
		QWidget *CreatePropertyWindow();
		/*! Updates the property widget with EC_OpenSimPrim information
		 *	\param prim EC_OpenSimPrim with the information to update
		 */
		void UpdatePropertyWindow(EC_OpenSimPrim *prim);
//$ END_MOD $
    private slots:
        bool IsTitleProperty(QtProperty *prop);
        //void SetCorrectedValue(QtProperty *prop, const QVariant &corrected_value);

    private:
        EC_OpenSimPrim *current_prim_;
        Helpers::UiHelper *ui_helper_;

        QtAbstractPropertyBrowser *property_browser_;
//$ BEGIN_MOD $
		//! Property widget
		QtAbstractPropertyBrowser* propertyWindow_;
//$ END_MOD $
    };
}

#endif