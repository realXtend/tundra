
#ifndef incl_QEC_PRIM_h
#define incl_QEC_PRIM_h
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

#include "RexUUID.h"

namespace PythonScript 
{
	using std::string;
	class QEC_Prim : public QObject
    {

        Q_OBJECT
		Q_PROPERTY(QMap map READ getValues)// WRITE setVsalue)
		Q_PROPERTY(QString name READ getName WRITE setName)
		Q_PROPERTY(QVariant uuid READ getUUID WRITE setUUID)
        
		public:
			QMap<QString, QVariant> map;
			QMap<QString, QVariant> getValues() { return map;}
			//void setValue(QMap<int, QString> map) { map = map; }

			QString getName() { return QString(name.c_str()); }
			void setName(QString nuname) { name = nuname.toStdString(); }

			QVariant getUUID() {return uuid;}
			void setUUID(QVariant nuuuid) {   uuid = nuuuid; }

        public slots:
			QString about() { return "swoot";}

		private:
			std::string name;
			QVariant uuid;

	};
}

#endif
