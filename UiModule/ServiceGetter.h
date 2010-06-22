// For conditions of distribution and use, see copyright notice in license.txt
///\file ServiceGetter.h
///\todo This file was used for a keyboard bindings configuration widget. It is no longer used,
///      and will be removed in the future.

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