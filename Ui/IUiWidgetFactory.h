/**
 *  For conditions of distribution and use, see copyright notice in license.txt
 *
 *  @file   IUiWidgetFactory.h
 *  @brief  Interface for QWidget factories.
 */

#ifndef incl_Ui_IUiWidgetFactory_h
#define incl_Ui_IUiWidgetFactory_h

#include <QObject>

class IUiWidgetFactory : public QObject
{
public:
    IUiWidgetFactory() {}
    virtual ~IUiWidgetFactory() {}

public slots:
    /// Returns the type of widgets this widget factory can create.
    virtual QString Type() const = 0;

    /// Creates a new widget of the given type.
    /*@param name The name to give for this widget.
      @param params List for specific factory parameters
    */
    virtual QWidget* CreateWidget(const QString &name, const QStringList &params = QStringList()) = 0;

};

#endif