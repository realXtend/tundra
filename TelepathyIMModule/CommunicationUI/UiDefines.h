// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Communication_UiDefines_h
#define incl_Communication_UiDefines_h

#include <QObject>
#include <QMap>
#include <QString>
#include <QIcon>
#include <QDateTime>

#include <QDebug>

namespace UiDefines
{
    class UiStates : public QObject
    {
    
    Q_OBJECT 
    
    public:
        enum ConnectionState
        {
            NoStateChange = 0,
            Disconnected = 1,
            Connecting = 2,
            Connected = 3,
            Exit = 4
        };
    
    };

    class PresenceStatus : public QObject
    {

    Q_OBJECT 
    
    public:

        static QString GetStatusForStatusCode(QString status_code)
        {
            QMap <QString,QString> code_to_status;
            code_to_status["available"] = "Available";
            code_to_status["chat"] = "Chatty";
            code_to_status["away"] = "Away";
            code_to_status["xa"] = "Extended Away";
            code_to_status["dnd"] = "Busy";
            code_to_status["hidden"] = "Hidden";
            code_to_status["offline"] = "Offline";
            code_to_status["unknown"] = "Unknown";
            return code_to_status[status_code];
        }

        static QString GetStatusCodeForStatus(QString status)
        {
            QMap <QString,QString> status_to_code;
            status_to_code["Available"] = "available";
            status_to_code["Chatty"] = "chat";
            status_to_code["Away"] = "away";
            status_to_code["Extended Away"] = "xa";
            status_to_code["Busy"] = "dnd";
            status_to_code["Hidden"] = "hidden";
            status_to_code["Offline"] = "offline";
            status_to_code["Unknown"] = "unknown";
            return status_to_code[status];
        }

        static QIcon GetIconForStatusCode(QString status_code)
        {
            QIcon icon;
            if ( QString::compare(status_code, QString("available"), Qt::CaseInsensitive) == 0 ||
                 QString::compare(status_code, QString("chat"), Qt::CaseInsensitive) == 0 )
                icon = QIcon(":/images/iconGreen.png");
            else if ( QString::compare(status_code, QString("offline"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString("hidden"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString("unknown"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString(""), Qt::CaseInsensitive) == 0 )
                icon = QIcon(":/images/iconGrey.png");
            else if ( QString::compare(status_code, QString("away"), Qt::CaseInsensitive) == 0 )
                icon = QIcon(":/images/iconYellow.png");
            else if ( QString::compare(status_code, QString("xa"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString("dnd"), Qt::CaseInsensitive) == 0 )
                icon = QIcon(":/images/iconOrange.png");
            else
            {
                qDebug() << "Unknows status code: " << status_code << endl;
                icon = QIcon(":/images/iconGrey.png");
            }
            return icon;
        }

        static QString GetImagePathForStatusCode(QString status_code)
        {
            QString path;
            if ( QString::compare(status_code, QString("available"), Qt::CaseInsensitive) == 0 ||
                 QString::compare(status_code, QString("chat"), Qt::CaseInsensitive) == 0 )
                path = ":/images/iconGreen.png";
            else if ( QString::compare(status_code, QString("offline"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString("hidden"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString("unknown"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString(""), Qt::CaseInsensitive) == 0 )
                path = ":/images/iconGrey.png";
            else if ( QString::compare(status_code, QString("away"), Qt::CaseInsensitive) == 0 )
                path = ":/images/iconYellow.png";
            else if ( QString::compare(status_code, QString("xa"), Qt::CaseInsensitive) == 0 ||
                      QString::compare(status_code, QString("dnd"), Qt::CaseInsensitive) == 0 )
                path = ":/images/iconOrange.png";
            else
                path = ":/images/iconGrey.png";
            return path;
        }

    };

    class TimeStampGenerator : public QObject
    {

    Q_OBJECT

    public:

        //! Generates a formatted timestamp of the current time and date and returns it as a QString
        static QString GenerateTimeStamp()
        {
            QDateTime time_stamp = QDateTime::currentDateTime();

            QString timestamp(QString("%1 %2").arg(time_stamp.date().toString("dd.MM.yyyy"),time_stamp.time().toString("hh:mm:ss")));
            return timestamp;
        }

        //! Formats an already existing QDateTime and returns it as a QString
        static QString FormatTimeStamp(QDateTime time)
        {
            QString timestamp(QString("%1 %2").arg(time.date().toString("dd.MM.yyyy"),time.time().toString("hh:mm:ss")));
            return timestamp;
        }
    };

}

#endif // incl_Communication_UiDefines_h