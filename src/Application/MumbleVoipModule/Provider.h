// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "MumbleFwd.h"
#include "IMumble.h"
#include "Session.h"
#include "ServerInfo.h"

#include <QObject>

namespace MumbleVoip
{
    class Provider : public IProvider
    {
        
    Q_OBJECT

    public:
        Provider(Framework* framework, Settings* settings);
        virtual ~Provider();

    public slots:
        virtual void Update(f64 frametime);

        /// IProvider override.
        virtual ISession* Session();

        /// IProvider override.
        virtual bool HasSession();

        /// IProvider override.
        virtual QString& Description();
        virtual QList<QString> Statistics();

        // Show the mumble audio config widget.
        virtual void ShowMicrophoneAdjustmentDialog();

    private slots:
        void CreateSession();
        void CloseSession();
        void ResetSession();

        void OnMicrophoneAdjustmentWidgetDestroyed();

    private:
        SessionPtr session_;
        Settings *settings_;

        Framework *framework_;
        QString description_;

        QWidget *microphone_adjustment_widget_;
    };
}
