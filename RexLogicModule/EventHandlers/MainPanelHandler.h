// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_RexLogic_MainPanelHandler_h
#define incl_RexLogic_MainPanelHandler_h

#include <QObject>

namespace RexLogic
{
    class RexLogicModule;

    class MainPanelHandler : public QObject
    {
        Q_OBJECT

    public:
        explicit MainPanelHandler(RexLogicModule *rexlogic);
        ~MainPanelHandler();

    public slots:
        void LogoutRequested();
        void QuitRequested();

    private:
        RexLogicModule *rexlogic_;
    };
}

#endif // incl_RexLogic_MainPanelHandler_h
