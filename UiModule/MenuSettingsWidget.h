//$ HEADER_NEW_FILE $
// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_MenuSettingsWidget_h
#define incl_UiModule_MenuSettingsWidget_h

#include "ui_MenuSettingsWidget.h"

#include <QWidget>
#include <QCheckBox>
#include <QString>

namespace UiServices
{
    class UiSceneService;

    //! Dialog for menu settings
    //! Managed by UiSceneServices, the one who controlles qdock and uiproxys
    class MenuSettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        //! Constructor.
        //! \param owner. UiSceneService that manages the panel
		explicit MenuSettingsWidget(UiSceneService *owner);

        //! Destructor.
        virtual ~MenuSettingsWidget();

        /*! Sets list of pannels that are going to be enabled/disabled (called by uisceneservice)
		*
		* \param panels List of panels available in Naali
		*/
        void SetPanelsList(QList<QString> &panels);

        //! Add checkboxes in the widget based in the list of panels
        void AddEffects();

    public slots:
		/*!To manage the click over the checkbox in the panel
		*
		* \param checked	Boolean to say if it is checked or not
		* \param name		Name of the checkbox
		*/
        void HandleSelection(bool checked, const QString &name);

    protected:
        void changeEvent(QEvent* e);

    private:
        //! Widget that this QWidget contains
        Ui::SelectionWidget widget_;

        //! UiSceneService; to call it when it has to change a menu position
		UiServices::UiSceneService *owner_;

		//List of panels
		QList<QString> panels_;
    };

    //! inherited to get name of the checkbox with toggle signal
    class NamedCheckBox: public QCheckBox
    {
        Q_OBJECT

    public:
        NamedCheckBox(const QString &text, QWidget *parent = 0);
        virtual ~NamedCheckBox();

    protected:
        void changeEvent(QEvent* e);

    public slots:
        void ButtonToggled(bool checked);

    signals:
        void Toggled(bool checked, const QString &name);
    };
}

#endif
