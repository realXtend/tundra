// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_ServiceGetter_h
#define incl_UiModule_ServiceGetter_h

#include "Foundation.h"
#include "KeyBindings.h"

#include <QObject>

namespace CoreUi
{
    class ServiceGetter : public QObject
    {

    Q_OBJECT

    public:
        ServiceGetter(Foundation::Framework *framework, QObject *parent = 0);

    public slots:
        void GetKeyBindings();
        void SetKeyBindings(Foundation::KeyBindings *bindings);
        void RestoreKeyBindings();
        void PublishChangedBindings(Foundation::KeyBindings *bindings);

    signals:
        void KeyBindingsChanged(Foundation::KeyBindings*);

    private:
        Foundation::Framework *framework_;
    };
}

#endif