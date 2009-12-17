
#ifndef incl_QEC_PRIM_h
#define incl_QEC_PRIM_h
#include <QObject>
#include <QMap>
#include <QString>
#include <QVariant>

#include "RexUUID.h"

using RexTypes::RexUUID;
namespace PythonScript 
{
	class QEC_Prim : public QObject
    {

        Q_OBJECT
		Q_PROPERTY(QMap map READ getValues)// WRITE setValue)
		Q_PROPERTY(QString name READ getName WRITE setName)
		Q_PROPERTY(QVariant uuid READ getUUID WRITE setUUID)
        
		public:
			QMap<QString, QVariant> map;
			QMap<QString, QVariant> getValues() { return map;}
			//void setValue(QMap<int, QString> map) { map = map; }
			QString getName() {return name;}
			void setName(QString nuname) { name = nuname; }
			QVariant getUUID() {return uuid;}
			void setUUID(QVariant nuuuid) { uuid = nuuuid; }

        public slots:
			QString about() { return "swoot";}

		private:
			QString name;
			QVariant uuid;

	};
}

#endif
