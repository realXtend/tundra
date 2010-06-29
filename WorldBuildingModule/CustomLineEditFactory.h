// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_WorldBuildingModule_CustomLineEditFactory_h
#define incl_WorldBuildingModule_CustomLineEditFactory_h

#include <QObject>
#include <QMap>
#define QT_QTPROPERTYBROWSER_IMPORT
#include <QtLineEditFactory>

class QtProperty;
class QLineEdit;

namespace WorldBuilding
{
    class CustomLineEditFactory : public QtAbstractEditorFactory<QtStringPropertyManager>
    {

    Q_OBJECT

    public:
        CustomLineEditFactory(QObject *parent = 0);
        virtual ~CustomLineEditFactory();

    protected:
        virtual void connectPropertyManager(QtStringPropertyManager *manager);
        virtual QWidget *createEditor(QtStringPropertyManager *manager, QtProperty *prop, QWidget *parent);
        virtual void disconnectPropertyManager(QtStringPropertyManager *manager);

    private slots:
        void EditorDestoryed(QObject *object);

    private:
        QMap<QtProperty*, QLineEdit*> property_to_editor_;

    };
}

#endif