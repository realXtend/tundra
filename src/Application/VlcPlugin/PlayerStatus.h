// For conditions of distribution and use, see copyright notice in license.txt

#pragma once

#include "CoreTypes.h"

#include <QObject>
#include <QString>
#include <QSize>

class PlayerStatus : public QObject
{

Q_OBJECT

public:
    enum StatusChangeType
    {
        NoChange = 0,
        MediaState,
        MediaProperty,
        MediaTime,
        MediaSource,
        MediaSize,
        PlayerError
    };

    PlayerStatus()
    {
        Reset();
    }

    PlayerStatus(const PlayerStatus &status)
    {
        playing = status.playing;
        stopped = status.stopped;
        paused = status.paused; 
        buffering = status.buffering;
        isSeekable = status.isSeekable;
        isPausable = status.isPausable;
        doRestart = status.doRestart;
        doStop = status.doStop;
        time = status.time;
        lenght = status.lenght;
        position = status.position;
        source = status.source;
        title = status.title;
        error = status.error;
        sourceSize = status.sourceSize;
        change = status.change;
    }

    void Reset()
    {
        playing = false;
        stopped = true;
        paused = false; 
        buffering = false;
        isSeekable = false;
        isPausable = false;
        doRestart = false;
        doStop = false;
        time = 0.0;
        lenght = 0.0;
        position = 0.0f;
        source = "";
        title = "";
        error = "";
        sourceSize = QSize(0,0);
        change = NoChange;
    }

    bool playing;
    bool stopped;
    bool paused; 
    bool buffering;
    bool isSeekable;
    bool isPausable;
    bool doRestart;
    bool doStop;

    boost::int64_t time;
    boost::int64_t lenght;
    float position;

    QString source;
    QString title;
    QString error;

    QSize sourceSize;

    StatusChangeType change;
};
