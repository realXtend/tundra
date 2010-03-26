// For conditions of distribution and use, see copyright notice in license.txt

#ifndef incl_Input_ConfigManager_h
#define incl_Input_ConfigManager_h

#include "Foundation.h"
#include "KeyBindings.h"

#include <QObject>
#include <QMap>
#include <QList>
#include <QStringList>
#include <QKeySequence>
#include <QVariant>

namespace Input
{
    class ConfigManager : public QObject
    {

    public:
        ConfigManager(QObject *parent);
        
        //! Return the current KeyBindings, either Default from bin or custom from user appdata
        Foundation::KeyBindings *GetUsedKeyBindings();
        
        //! Parse a config with group and return KeyBindings
        Foundation::KeyBindings *ParseConfig(QString group);

        //! Write a custom config with in param KeyBindings
        void WriteCustomConfig(Foundation::KeyBindings *bindings);

        void ClearUserConfig();

    private:
        //! Check if custom bindings have been set
        bool CustomConfigDefined();
    
        QString default_config_;
        QString custom_config_;

    };
}

#endif