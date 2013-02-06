// For conditions of distribution and use, see copyright notice in LICENSE

#pragma once

#include <QWidget>
#include "ui_CaveSettings.h"
#include "CAVEViewSettings.h"
#include "CAVEViewSettingsAdvanced.h"
#include <QSignalMapper>

class Framework;

namespace CAVEStereo
{
    class CAVESettingsWidget: public QWidget, private Ui::CAVESettings 
    {
        Q_OBJECT
    
    public:
        CAVESettingsWidget(Framework* framework, QWidget* parent = 0);

    signals:
        void ToggleCAVE(bool);
        void NewCAVEViewRequested(const QString& name, Ogre::Vector3 &tl, Ogre::Vector3 &bl, Ogre::Vector3 &br, Ogre::Vector3 &eye);
        void GetCAVEViewProjParams(const QString& name, Ogre::Vector3 &tl, Ogre::Vector3 &bl, Ogre::Vector3 &br, Ogre::Vector3 &eye);
        void ModifyCAVEViewProjParams(const QString& name, Ogre::Vector3 &tl, Ogre::Vector3 &bl, Ogre::Vector3 &br, Ogre::Vector3 &eye);
        void RemoveCAVEView(const QString& name);
        void NewCAVEPanoramaViewRequested(const QString& name, Ogre::Vector3 &tl, Ogre::Vector3 &bl, Ogre::Vector3 &br, Ogre::Vector3 &eye, int n);

    public slots:
        void CAVEButtonToggled(bool v);
        void AddNewCAVEView();
        void AddNewCAVEViewAdvanced();
        void BNAVE();
        void VCAVE();
        void MiniCAVE();
        void Panorama();
        void AddViewToUi(const QString& name);
        void ModifyViewPressed(QString name);
        void DeleteViewPressed(QString name);
        void ShowCaveWindow();

    private:
        QString GetNextName();
        Framework* framework_;
        CAVEViewSettings settings_dialog_;
        CAVEViewSettingsAdvanced settings_dialog_advanced_;
        int next_view_num_;
        QString view_prefix_;
        QSignalMapper modmapper_;
        QSignalMapper remmapper_;
    };
}
