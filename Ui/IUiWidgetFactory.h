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
    virtual QString Type() const = 0;

    virtual QWidget* CreateWidget(const QString &name, const QStringList &params = QStringList()) = 0;

};

#endif