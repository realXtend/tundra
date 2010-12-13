// For conditions of distribution and use, see copyright notice in license.txt

#include "StableHeaders.h"
#include "DebugOperatorNew.h"

#include "SettingsWidget.h"
#include "TtsServiceInterface.h"
#include <QSettings>
#include "MemoryLeakCheck.h"

namespace Tts
{
    const char Settings::SETTINGS_HEADER_[] = "configuration/Tts";

    void Settings::Load()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        playOwnChatMessages = settings.value("Tts/play_own_chat_messages", false).toBool();
        playOtherChatMessages = settings.value("Tts/play_other_chat_messages", false).toBool();
        playNotificationMessages = settings.value("Tts/play_notifications_messages", false).toBool();
        ownVoice = settings.value("Tts/own_voice", "").toString();
        testVoice = settings.value("Tts/test_voice", "").toString();
        otherDefaultVoice = settings.value("Tts/other_default_voice", "").toString();
        notificationVoice = settings.value("Tts/notification_voice", "").toString();
        testPhrase = settings.value("Tts/test_phrase", "Hello world!").toString();
    }

    void Settings::Save()
    {
        QSettings settings(QSettings::IniFormat, QSettings::UserScope, APPLICATION_NAME, SETTINGS_HEADER_);
        settings.setValue("Tts/play_own_chat_messages", playOwnChatMessages);
        settings.setValue("Tts/play_other_chat_messages", playOtherChatMessages);
        settings.setValue("Tts/play_notifications_messages", playNotificationMessages);
        settings.setValue("Tts/own_voice", ownVoice);
        settings.setValue("Tts/test_voice", testVoice);
        settings.setValue("Tts/other_default_voice", otherDefaultVoice);
        settings.setValue("Tts/notification_voice", notificationVoice);
        settings.setValue("Tts/test_phrase", testPhrase);
        settings.sync();
    }

    SettingsWidget::SettingsWidget(Foundation::Framework* framework):
        QWidget(),
        framework_(framework)
    {
        setupUi(this);
        UpdateVoiceOptions();
        LoadSettings();
        connect(testButton, SIGNAL(clicked()), this, SLOT(TestVoice()) );
        connect(playOwnMessagesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SaveSettings()) );
        connect(playOtherMessagesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SaveSettings()) );
        connect(playNotificationMessagesCheckBox, SIGNAL(stateChanged(int)), this, SLOT(SaveSettings()) );
        connect(testPhraseText, SIGNAL(textChanged(const QString &)), this, SLOT(SaveSettings()) );
        connect(ownVoice, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(SaveSettings()) );
        connect(defaultVoiceForTextChat, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(SaveSettings()) );
        connect(notificationVoice, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(SaveSettings()) );
        connect(testVoice, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(SaveSettings()) );
    }

    SettingsWidget::~SettingsWidget()
    {
        SaveSettings();
    }

    void SettingsWidget::SaveSettings()
    {
        settings_.playOwnChatMessages = playOwnMessagesCheckBox->isChecked();
        settings_.playOtherChatMessages = playOtherMessagesCheckBox->isChecked();
        settings_.playNotificationMessages = playNotificationMessagesCheckBox->isChecked();
        settings_.ownVoice = ownVoice->currentText();
        settings_.testVoice = testVoice->currentText();
        settings_.otherDefaultVoice = defaultVoiceForTextChat->currentText();
        settings_.notificationVoice = notificationVoice->currentText();
        settings_.testPhrase = testPhraseText->text();
        settings_.Save();
    }

    void SettingsWidget::UpdateVoiceOptions()
    {
        Tts::TtsServiceInterface* tts_service = framework_->GetService<Tts::TtsServiceInterface>();
		if (!tts_service)
			return;

        ownVoice->addItems(tts_service->GetAvailableVoices());
        defaultVoiceForTextChat->addItems(tts_service->GetAvailableVoices());
        notificationVoice->addItems(tts_service->GetAvailableVoices());
        testVoice->addItems(tts_service->GetAvailableVoices());
    }

    // Set given text to currentText of the given combobox if the text is an option
    // otherwise currentText is not changed.
    void SetQComboBoxCurrentText(QComboBox* comboBox, QString text)
    {
        if (!comboBox)
            return;
        for (int i = 0; i < comboBox->count(); ++i)
        {
            if (comboBox->itemText(i) == text)
            {
                comboBox->setCurrentIndex(i);
                return;
            }
        }
    }

    void SettingsWidget::LoadSettings()
    {
        settings_.Load();
        if (settings_.playOwnChatMessages)
            playOwnMessagesCheckBox->setCheckState(Qt::Checked);
        else
            playOwnMessagesCheckBox->setCheckState(Qt::Unchecked);
        if (settings_.playOtherChatMessages)
            playOtherMessagesCheckBox->setCheckState(Qt::Checked);
        else
            playOtherMessagesCheckBox->setCheckState(Qt::Unchecked);
        if (settings_.playNotificationMessages)
            playNotificationMessagesCheckBox->setCheckState(Qt::Checked);
        else
            playNotificationMessagesCheckBox->setCheckState(Qt::Unchecked);

        SetQComboBoxCurrentText(ownVoice, settings_.ownVoice);
        SetQComboBoxCurrentText(notificationVoice, settings_.notificationVoice);
        SetQComboBoxCurrentText(defaultVoiceForTextChat, settings_.otherDefaultVoice);
        SetQComboBoxCurrentText(testVoice, settings_.testVoice);

        testPhraseText->setText(settings_.testPhrase);
    }
        
    void SettingsWidget::TestVoice()
    {
        QString text = testPhraseText->displayText();
        QString voice = testVoice->currentText();

        Tts::TtsServiceInterface* tts_service = framework_->GetService<Tts::TtsServiceInterface>();
		if (!tts_service)
			return;

        tts_service->Text2Speech(text, voice);
    }

} // Tts
