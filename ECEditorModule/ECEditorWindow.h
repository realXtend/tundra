// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_ECEditorModule_ECEditorWindow_h
#define incl_ECEditorModule_ECEditorWindow_h

#include "Framework.h"

#include <QWidget>

namespace ECEditor
{
    class ECEditorWindow : public QWidget
    {
        Q_OBJECT
        
    public:
        ECEditorWindow(Foundation::Framework* framework);
        ~ECEditorWindow();
        
        Foundation::Framework *framework_;
        
    private:
        void Initialize();
        
        QWidget* contents_;
    };
}

#endif
