// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_UiModule_IToolWidget_h
#define incl_UiModule_IToolWidget_h

#include <QObject>
#include <QWidget>

namespace CoreUi
{
    /// @todo Move to separate file
    class IToolWidget: public QWidget
    {
        Q_OBJECT
    public:
        virtual ~IToolWidget() {};

    public slots:
        /// 
        virtual void Minimize() = 0;
        /// 
        virtual void Maximize() = 0;

    public:
//        virtual bool IsMinimized() = 0;
    };

} // namespace CoreUi

#endif // incl_UiModule_IToolWidget_h
