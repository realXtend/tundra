// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_KeyDataManager_h
#define incl_Input_KeyDataManager_h

#include "Foundation.h"
#include "KeyBindings.h"

#include <QObject>
#include <QMap>
#include <QMultiMap>
#include <QList>
#include <QPair>
#include <QKeySequence>
#include <QVariant>
#include <QStringList>

namespace Input
{
    class KeyDataManager : public QObject
    {
    
    Q_OBJECT

    public:
        KeyDataManager(QObject *parent);
        
    public slots:
        //! Generate the default naali config
        void GenerateAndParseDefaultConfig(KeyBindingMap **default_bindings);

        //! Check if custom bindings have been set
        bool CustomConfigDefined();
        
        //! Update configs
        void WriteCustomConfig(QMap<QString, QList<QVariant> > key_map);
        
        //! Parse custom config
        void ParseCustomConfig();

        //! Get the current key list in the WorldInputModules internal format
        QMultiMap<std::pair<int,int>, QKeySequence> GetInternalFormatList();

        //! Get the current key map QString to QKeySequence (aka QString inside QVariant)
        QMap<QString, QList<QVariant> > GetCurrentKeyMap() { return current_key_map_; }

        //! Get the Naali default bindings
        QMap<QString, QList<QVariant> > GetDefaultKeyMap(){ return naali_default_key_map_; }

        //! Return config key list
        QStringList GetConfigKeys() { return config_keys_; }

        //! Revert everything = delete custom config, set current key map to naali defaults
        void RevertEverythingToDefault();

    private slots:
        QString NameForEvent(int event_id);
        std::pair<int,int> EventPairForName(QString name);
    
    private:
        QString default_config_;
        QString custom_config_;
        QStringList config_keys_;

        QMap<QString, QList<QVariant> > current_key_map_;
        QMap<QString, QList<QVariant> > naali_default_key_map_;

    signals:
        void KeyBindingsChanged(QMultiMap<std::pair<int,int>, QKeySequence>);

    };
}

#endif