// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Interfaces_UiWidgetProperties_h
#define incl_Interfaces_UiWidgetProperties_h

#include <QObject>
#include <QSize>
#include <QPointF>
#include <QString>
#include <QMap>

namespace UiServices
{
    class UiWidgetProperties : public QObject
    {
        Q_OBJECT
        // READ
        //read-write now for the workaround in py for constructors not working there yet
        Q_PROPERTY(QString name_ READ GetName)

        // READ & WRITE
        Q_PROPERTY(QString name_ READ GetName WRITE SetName)
        Q_PROPERTY(QString group_ READ GetMenuGroup WRITE SetMenuGroup)

    public:
        /*! Creates new properties according to the widget_type. For types check the enum documentation in the header.
         *  Default type is ModuleWidget, will have window frames and a control button is added to the main panel for show/hide
         *  \param QString widget_name, name of the widget
         *  \param UiServices::WidgetType widget_type, type of the window
         */
        UiWidgetProperties(const QString &name, const QString &icon = "./data/ui/images/menus/edbutton_LSCENE_normal.png") :
            name_(name),
            icon_(icon),
            group_("")
        {
        }

        //! Copy-constructor.
        UiWidgetProperties(const UiWidgetProperties &rhs) :
            name_(rhs.GetName()),
            group_(rhs.GetMenuGroup()),
            icon_(rhs.GetIcon())
        {
        }

        //! Destructor.
        ~UiWidgetProperties() {}

    public slots:
        //! Getters for properties
        const QString GetName() const { return name_; }
        QString GetIcon() const { return icon_; }
        QString GetMenuGroup() const { return group_; }

        //! Setters for properties
        void SetName(const QString &name) { name_ = name; }
        void SetMenuGroup(const QString &group) { group_ = group; }
        void SetIcon(const QString &icon) { icon_ = icon; }

    private:
        /// Menu entry name.
        QString name_;

        /// File path to icon.
        QString icon_;

        /// Menu group.
        QString group_; 
    };
}

#endif // incl_UiModule_UiWidgetProperties_h
