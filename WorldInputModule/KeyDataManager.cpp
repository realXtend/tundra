// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "KeyDataManager.h"
#include "InputEvents.h"

#include <QFile>
#include <QSettings>

#include <QDebug>

namespace Input
{
    KeyDataManager::KeyDataManager(QObject *parent) :
        QObject(parent),
        default_config_("bindings/default"),
        custom_config_("bindings/current"),
        config_keys_()
    {
        config_keys_ << "move.back" << "move.forward" << "move.up" << "move.down"  << "move.left" << "move.right"
                     << "naali.delete" << "naali.undo" << "naali.object.link" << "naali.object.unlink"
                     << "python.duplicate.drag" << "python.object.toggle.move" << "python.object.toggle.scale" << "python.restart" << "python.run"
                     << "toggle.camera" << "toggle.console" << "toggle.fly" 
                     << "rotate.left" << "rotate.right"
                     << "zoom.in" << "zoom.out";
    }

    void KeyDataManager::GenerateAndParseDefaultConfig(KeyBindingMap **default_bindings)
    {
        // Do this only once on startup
        if (naali_default_key_map_.count() > 0)
            return;
        
        // Open up the default config
        QSettings default_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", default_config_);
        if (!default_settings.isWritable())
            return;
        default_settings.clear();
        current_key_map_.clear();

        // Go trought the code set bindings
        KeyBindingMap::iterator iter = (*default_bindings)->begin();
        KeyBindingMap::iterator end = (*default_bindings)->end();
        while (iter != end)
        {
            QKeySequence seq = iter->first;
            std::pair<int,int> ids = iter->second;
            iter++;

            QString identifier = NameForEvent(ids.first);
            if (identifier.isEmpty())
                continue;

            if (current_key_map_.contains(identifier))
                current_key_map_[identifier].append(seq);
            else
            {
                QList<QVariant> sequence_list;
                sequence_list.append(seq);
                current_key_map_[identifier] = sequence_list;
            } 
        }

        // Save the default mappings to config
        default_settings.beginGroup("Bindings.Default");
        foreach(QString identifier, current_key_map_.keys())
        {
            if (current_key_map_[identifier].count() > 1)
                default_settings.setValue(identifier, current_key_map_[identifier]);
            else
                default_settings.setValue(identifier, QKeySequence(current_key_map_[identifier].at(0).toString()));
        }
        default_settings.endGroup();
        default_settings.sync();

        // Store the default locally for when user wants to revert bindings
        naali_default_key_map_ = current_key_map_;
    }

    bool KeyDataManager::CustomConfigDefined()
    {
        QSettings custom_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", custom_config_);
        if (custom_settings.childGroups().count() == 0)
            return false;
        return true;
    }

    void KeyDataManager::WriteCustomConfig(QMap<QString, QList<QVariant> > key_map)
    {
        QSettings custom_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", custom_config_);
        if (!custom_settings.isWritable())
            return;

        current_key_map_ = key_map;

        custom_settings.beginGroup("Bindings.Custom");
        foreach(QString identifier, key_map.keys())
        {
            if (key_map[identifier].count() > 1)
                custom_settings.setValue(identifier, key_map[identifier]);
            else
                custom_settings.setValue(identifier, QKeySequence(key_map[identifier].at(0).toString()));
        }
        custom_settings.endGroup();
        custom_settings.sync();

        emit KeyBindingsChanged(GetInternalFormatList());
    }

    void KeyDataManager::ParseCustomConfig()
    {
        QSettings custom_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", custom_config_);
        if (!custom_settings.isWritable())
            return;
        current_key_map_.clear();

        QString group = "Bindings.Custom";
        foreach (QString identifier, config_keys_)
        {
            QVariant current = custom_settings.value(group + "/" + identifier);
            if (current.isNull())
                continue;

            QStringList keys_seq_list = current.toStringList();
            foreach (QString seq, keys_seq_list)
            {
                if (current_key_map_.contains(identifier))
                    current_key_map_[identifier].append(QVariant(seq));
                else
                {
                    QList<QVariant> seq_list;
                    seq_list.append(QVariant(seq));
                    current_key_map_[identifier] = seq_list;
                }
            }
        }
    }

    QMultiMap<std::pair<int,int>, QKeySequence> KeyDataManager::GetInternalFormatList()
    {
        QMultiMap<std::pair<int,int>, QKeySequence> internal_format_map;

        foreach (QString identifier, current_key_map_.keys())
        {
            QList<QVariant> value_list = current_key_map_[identifier];
            foreach(QVariant value, value_list)
            {
                QKeySequence seq(value.toString());
                std::pair<int,int> id_pair = EventPairForName(identifier);
                if (id_pair.first != 0)
                    internal_format_map.insert(id_pair, seq);
            }
        }

        return internal_format_map;
    }

    void KeyDataManager::RevertEverythingToDefault()
    {
        QSettings custom_settings(QSettings::IniFormat, QSettings::UserScope, "realXtend", custom_config_);
        if (custom_settings.isWritable())
            custom_settings.clear();

        current_key_map_ = naali_default_key_map_;
        emit KeyBindingsChanged(GetInternalFormatList());
    }

    // Private

    QString KeyDataManager::NameForEvent(int event_id)
    {
        QString name;
        switch (event_id)
        {
            // Fly
            case Input::Events::TOGGLE_FLYMODE:
                name = "toggle.fly";
                break;

            // Camera
            case Input::Events::SWITCH_CAMERA_STATE:
                name = "toggle.camera";
                break;

            // Console
            case Input::Events::SHOW_DEBUG_CONSOLE:
                name = "toggle.console";
                break;

            // Zoom
            case Input::Events::ZOOM_IN_PRESSED:
                name = "zoom.in";
                break;
            case Input::Events::ZOOM_OUT_PRESSED:
                name = "zoom.out";
                break;

            // Move
            case Input::Events::MOVE_FORWARD_PRESSED:
                name = "move.forward";
                break;
            case Input::Events::MOVE_BACK_PRESSED:
                name = "move.back";
                break;
            case Input::Events::MOVE_LEFT_PRESSED:
                name = "move.left";
                break;
            case Input::Events::MOVE_RIGHT_PRESSED:
                name = "move.right";
                break;
            case Input::Events::MOVE_UP_PRESSED:
                name = "move.up";
                break;
            case Input::Events::MOVE_DOWN_PRESSED:
                name = "move.down";
                break;

            // Rotate
            case Input::Events::ROTATE_LEFT_PRESSED:
                name = "rotate.left";
                break;
            case Input::Events::ROTATE_RIGHT_PRESSED:
                name = "rotate.right";
                break;

            // Naali
            case Input::Events::NAALI_DELETE:
                name = "naali.delete";
                break;
            case Input::Events::NAALI_UNDO:
                name = "naali.undo";
                break;
            case Input::Events::NAALI_OBJECTLINK:
                name = "naali.object.link";
                break;
            case Input::Events::NAALI_OBJECTUNLINK:
                name = "naali.object.unlink";
                break;

            // Python
            case Input::Events::PY_RUN_COMMAND:
                name = "python.run";
                break;
            case Input::Events::PY_RESTART:
                name = "python.restart";
                break;
            case Input::Events::PY_DUPLICATE_DRAG:
                name = "python.duplicate.drag";
                break;
            case Input::Events::PY_OBJECTEDIT_TOGGLE_MOVE:
                name = "python.object.toggle.move";
                break;
            case Input::Events::PY_OBJECTEDIT_TOGGLE_SCALE:
                name = "python.object.toggle.scale";
                break;

            // Unknown
            default:
                name = "";
                break;
        }
        return name;
    }

    std::pair<int,int> KeyDataManager::EventPairForName(QString name)
    {
        std::pair<int,int> id_pair;

        if (name == "toggle.fly")
        {
            id_pair.first = Input::Events::TOGGLE_FLYMODE;
            id_pair.second = 0;
        }
        else if (name == "toggle.camera")
        {
            id_pair.first = Input::Events::SWITCH_CAMERA_STATE;
            id_pair.second = 0;
        }
        else if (name == "toggle.console")
        {
            id_pair.first = Input::Events::SHOW_DEBUG_CONSOLE;
            id_pair.second = 0;
        }
        else if (name == "zoom.in")
        {
            id_pair.first = Input::Events::ZOOM_IN_PRESSED;
            id_pair.second = Input::Events::ZOOM_IN_RELEASED;
        }
        else if (name == "zoom.out")
        {
            id_pair.first = Input::Events::ZOOM_OUT_PRESSED;
            id_pair.second = Input::Events::ZOOM_OUT_RELEASED;
        }
        else if (name == "move.forward")
        {
            id_pair.first = Input::Events::MOVE_FORWARD_PRESSED;
            id_pair.second = Input::Events::MOVE_FORWARD_RELEASED;
        }
        else if (name == "move.back")
        {
            id_pair.first = Input::Events::MOVE_BACK_PRESSED;
            id_pair.second = Input::Events::MOVE_BACK_RELEASED;
        }
        else if (name == "move.left")
        {
            id_pair.first = Input::Events::MOVE_LEFT_PRESSED;
            id_pair.second = Input::Events::MOVE_LEFT_RELEASED;
        }
        else if (name == "move.right")
        {
            id_pair.first = Input::Events::MOVE_RIGHT_PRESSED;
            id_pair.second = Input::Events::MOVE_RIGHT_RELEASED;
        }
        else if (name == "move.up")
        {
            id_pair.first = Input::Events::MOVE_UP_PRESSED;
            id_pair.second = Input::Events::MOVE_UP_RELEASED;
        }
        else if (name == "move.down")
        {
            id_pair.first = Input::Events::MOVE_DOWN_PRESSED;
            id_pair.second = Input::Events::MOVE_DOWN_RELEASED;
        }
        else if (name == "rotate.left")
        {
            id_pair.first = Input::Events::ROTATE_LEFT_PRESSED;
            id_pair.second = Input::Events::ROTATE_LEFT_RELEASED;
        }
        else if (name == "rotate.right")
        {
            id_pair.first = Input::Events::ROTATE_RIGHT_PRESSED;
            id_pair.second = Input::Events::ROTATE_RIGHT_RELEASED;
        }
        else if (name == "naali.delete")
        {
            id_pair.first = Input::Events::NAALI_DELETE;
            id_pair.second = 0;
        }
        else if (name == "naali.undo")
        {
            id_pair.first = Input::Events::NAALI_UNDO;
            id_pair.second = 0;
        }
        else if (name == "naali.object.link")
        {
            id_pair.first = Input::Events::NAALI_OBJECTLINK;
            id_pair.second = 0;
        }
        else if (name == "naali.object.unlink")
        {
            id_pair.first = Input::Events::NAALI_OBJECTUNLINK;
            id_pair.second = 0;
        }
        else if (name == "python.run")
        {
            id_pair.first = Input::Events::PY_RUN_COMMAND;
            id_pair.second = 0;
        }
        else if (name == "python.restart")
        {
            id_pair.first = Input::Events::PY_RESTART;
            id_pair.second = 0;
        }
        else if (name == "python.duplicate.drag")
        {
            id_pair.first = Input::Events::PY_DUPLICATE_DRAG;
            id_pair.second = 0;
        }
        else if (name == "python.object.toggle.move")
        {
            id_pair.first = Input::Events::PY_OBJECTEDIT_TOGGLE_MOVE;
            id_pair.second = 0;
        }
        else if (name == "python.object.toggle.scale")
        {
            id_pair.first = Input::Events::PY_OBJECTEDIT_TOGGLE_SCALE;
            id_pair.second = 0;
        }
        else
        {
            id_pair.first = 0;
            id_pair.second = 0;
        }

        return id_pair;
    }
}