// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_NoteCardModule_NoteCardManager_h
#define incl_NoteCardModule_NoteCardManager_h

#include "Framework.h"

#include <QWidget>

namespace NoteCard
{
    class NoteCardManager : public QWidget
    {
        Q_OBJECT
        
    public:
        NoteCardManager(Foundation::Framework* framework);
        ~NoteCardManager();

    private:
        void Initialize();

        Foundation::Framework *framework_;
        QWidget* contents_;
    };
}

#endif
