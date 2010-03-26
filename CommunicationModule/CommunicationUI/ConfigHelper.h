// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_ConfigHelper_h
#define incl_Communication_ConfigHelper_h

//#include <Foundation.h>
#include <QObject>
#include <QMap>

namespace Foundation
{
    class Framework;
}

namespace Ui 
{
    class LoginWidget;
}

namespace UiHelpers
{
    class ConfigHelper : public QObject
    {

    Q_OBJECT
    
    public:
        explicit ConfigHelper(Foundation::Framework *framework);
        virtual ~ConfigHelper();

        void ReadLoginData(Ui::LoginWidget *login_ui);
        void SetPreviousData(Ui::LoginWidget *login_ui, QMap<QString,QString> data_map);
        void SaveLoginData(QMap<QString,QString> data_map);

    private:
        Foundation::Framework *framework_;
    };
}

#endif // incl_Communication_ConfigHelper_h